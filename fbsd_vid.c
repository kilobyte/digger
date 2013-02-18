/*
 * ---------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, (c) Poul-Henning Kamp): Maxim
 * Sobolev <sobomax@altavista.net> wrote this file. As long as you retain
 * this  notice you can  do whatever you  want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 *
 * Maxim Sobolev
 * ---------------------------------------------------------------------------
 */

#include <sys/fbio.h>
#include <sys/kbio.h>
#include <sys/consio.h>
#include <vgl.h>
#include <signal.h>
#include <osreldate.h>

#include "def.h"
#include "hardware.h"
#include "title_gz.h"

extern uint8_t   *vgatable[];
extern uint8_t   *ascii2vga[];

uint8_t         **sprites = vgatable;
uint8_t         **alphas = ascii2vga;

int16_t           xratio = 2;
int16_t           yratio = 2;
int16_t           yoffset = 0;
int16_t           hratio = 2;
int16_t           wratio = 2;
#define virt2scrx(x) (x*xratio)
#define virt2scry(y) (y*yratio+yoffset)
#define virt2scrw(w) (w*wratio)
#define virt2scrh(h) (h*hratio)

typedef uint8_t   palette[3];

/* palette1, normal intensity */
palette         vga16_pal1[] = \
{
    {0, 0, 0}, {225, 0, 0}, {0, 225, 0}, {225, 225, 0}, {0, 0, 225}, {225, 0, 225}, {0, 210, 225}, \
    {225, 225, 225}, {210, 210, 210}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, \
    {0, 255, 255}, {255, 255, 255}
};
/* palette1, high intensity */
palette         vga16_pal1i[] = \
{
    {0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {255, 0, 255}, {0, 225, 255}, \
    {240, 240, 240}, {225, 225, 225}, {255, 225, 225}, {225, 255, 225}, {255, 255, 225}, \
    {225, 225, 255}, {255, 225, 255}, {225, 255, 255}, {255, 255, 255}
};
/* palette2, normal intensity */
palette         vga16_pal2[] = \
{
    {0, 0, 0}, {0, 225, 0}, {0, 0, 225}, {0, 210, 225}, {225, 0, 0}, {225, 225, 0}, {225, 0, 225}, \
    {225, 225, 225}, {210, 210, 210}, {0, 255, 0}, {0, 0, 255}, {0, 255, 255}, {255, 0, 0}, {255, 255, 0}, \
    {255, 0, 255}, {255, 255, 255}
};
/* palette2, high intensity */
palette         vga16_pal2i[] = \
{
    {0, 0, 0}, {0, 255, 0}, {0, 0, 255}, {0, 225, 255}, {255, 0, 0}, {255, 255, 0}, {255, 0, 255}, \
    {240, 240, 240}, {225, 225, 225}, {225, 255, 225}, {225, 225, 255}, {225, 255, 255}, \
    {255, 225, 225}, {255, 255, 225}, {255, 225, 255}, {255, 255, 255}
};

palette        *npalettes[] = {vga16_pal1, vga16_pal2};
palette        *ipalettes[] = {vga16_pal1i, vga16_pal2i};
int16_t           currpal = 0;

#if __FreeBSD_version < 400000
VGLBitmap      *
VGLBitmapCreate(int type, int xsize, int ysize, byte * bits)
{
    VGLBitmap      *object;

    if (type != MEMBUF)
        return NULL;
    if (xsize < 0 || ysize < 0)
        return NULL;
    object = (VGLBitmap *) malloc(sizeof(*object));
    if (object == NULL)
        return NULL;
    object->Type = type;
    object->Xsize = xsize;
    object->Ysize = ysize;
    object->Bitmap = bits;
    return object;
}

void
VGLBitmapDestroy(VGLBitmap * object)
{
    if (object->Bitmap)
        free(object->Bitmap);
    free(object);
}

int
VGLBitmapAllocateBits(VGLBitmap * object)
{
    object->Bitmap = (byte *) malloc(object->Xsize * object->Ysize);
    if (object->Bitmap == NULL)
        return -1;
    return 0;
}
#endif

VGLBitmap      *
ch2bmap(uint8_t * sprite, int16_t w, int16_t h)
{
    int16_t           realw, realh;
    VGLBitmap      *tmp;

    realw = virt2scrw(w * 4);
    realh = virt2scrh(h);
    tmp = VGLBitmapCreate(MEMBUF, realw, realh, NULL);
    tmp->Bitmap = sprite;

    return (tmp);
}

void
graphicsoff(void)
{
    VGLEnd();
}

void
vgainit(void)
{
    if (geteuid() != 0)
    {
        fprintf(stderr, "The current graphics console architecture only permits " \
                "super-user to access it, therefore you either have to obtain such permissions" \
                "or ask your sysadmin to put set-user-id on digger executable.\n");
        exit(1);
    }
    if (VGLInit(SW_VESA_CG640x400) != 0)
    {
        fprintf(stderr, "WARNING! Could not initialise VESA mode. " \
                "Trying to fallback to the VGA 640x480 mode\n");
        if (VGLInit(SW_CG640x480) == 0)
            yoffset = 40;               /* Center the image */
        else
        {
            fprintf(stderr, "WARNING! Could not initialise VGA mode either. " \
                    "Please check your kernel.\n");
            exit(1);
        }
    }
    /*
     * Since the VGL library doesn't provide a default way to restore console
     * and keyboard after uncatched by the program signal, we should try to
     * catch at least what we could catch and pray to God that he would not
     * send SIGKILL to us.
     */
    signal(SIGHUP, catcher);
    signal(SIGINT, catcher);
    signal(SIGQUIT, catcher);
    signal(SIGABRT, catcher);
    signal(SIGTERM, catcher);
    signal(SIGSEGV, catcher);
    signal(SIGBUS, catcher);
    signal(SIGILL, catcher);
}

void
vgaclear(void)
{
    VGLClear(VGLDisplay, 0);
}

void
setpal(palette * pal)
{
    int16_t           i;

    for (i = 0; i < 16; i++)
    {
        VGLSetPaletteIndex(i, (pal[i])[2], \
                           (pal[i])[1], (pal[i])[0]);
    }
}

void
vgainten(int16_t inten)
{
    if (inten == 1)
        setpal(ipalettes[currpal]);
    else
        setpal(npalettes[currpal]);
}

void
vgapal(int16_t pal)
{
    setpal(npalettes[pal]);
    currpal = pal;
}

void
vgaputi(int16_t x, int16_t y, uint8_t * p, int16_t w, int16_t h)
{
    VGLBitmap      *tmp;
    int16_t           realx, realy, realh, realw;

    realx = virt2scrx(x);
    realy = virt2scry(y);
    realw = virt2scrw(w * 4);
    realh = virt2scrh(h);

    memcpy(&tmp, p, (sizeof(VGLBitmap *)));
    VGLBitmapCopy(tmp, 0, 0, VGLDisplay, realx, realy, realw, realh);
}

void
vgageti(int16_t x, int16_t y, uint8_t * p, int16_t w, int16_t h)
{
    VGLBitmap      *tmp;
    int16_t           realx, realy, realh, realw;

    memcpy(&tmp, p, (sizeof(VGLBitmap *)));
    if (tmp != NULL)
        VGLBitmapDestroy(tmp);  /* Destroy previously allocated bitmap */

    realx = virt2scrx(x);
    realy = virt2scry(y);
    realw = virt2scrw(w * 4);
    realh = virt2scrh(h);

    tmp = VGLBitmapCreate(MEMBUF, realw, realh, NULL);
    VGLBitmapAllocateBits(tmp);
    VGLClear(tmp, 0);
    VGLBitmapCopy(VGLDisplay, realx, realy, tmp, 0, 0, realw, realh);

    memcpy(p, &tmp, (sizeof(VGLBitmap *)));
}

int16_t
vgagetpix(int16_t x, int16_t y)
{
    VGLBitmap      *tmp = NULL;
    uint16_t           xi, yi;
    uint16_t           i = 0;
    int16_t           rval = 0;
    if ((x > 319) || (y > 199))
        return (0xff);
    vgageti(x, y, (uint8_t *) & tmp, 1, 1);
    for (yi = 0; yi < tmp->Ysize; yi++)
        for (xi = 0; xi < tmp->Xsize; xi++)
            if (tmp->Bitmap[i++])
                rval |= 0x80 >> xi;

    VGLBitmapDestroy(tmp);

    return (rval & 0xee);
}

void
vgaputim(int16_t x, int16_t y, int16_t ch, int16_t w, int16_t h)
{
    VGLBitmap      *tmp;
    VGLBitmap      *mask;
    VGLBitmap      *scr = NULL;
    int16_t           realsize;
    int16_t           i;

    tmp = ch2bmap(sprites[ch * 2], w, h);
    mask = ch2bmap(sprites[ch * 2 + 1], w, h);
    vgageti(x, y, (uint8_t *) & scr, w, h);
    realsize = scr->Xsize * scr->Ysize;
    for (i = 0; i < realsize; i++)
        if (tmp->Bitmap[i] != 0xff)
            scr->Bitmap[i] = (scr->Bitmap[i] & mask->Bitmap[i]) | \
                             tmp->Bitmap[i];

    vgaputi(x, y, (uint8_t *) & scr, w, h);
    tmp->Bitmap = NULL;         /* We should NULL'ify these ppointers, or the
                                 * VGLBitmapDestroy */
    mask->Bitmap = NULL;        /* will shoot itself in the foot by trying to
                                 * dellocate statically */
    VGLBitmapDestroy(tmp);      /* allocated arrays */
    VGLBitmapDestroy(mask);
    VGLBitmapDestroy(scr);
}

void
vgawrite(int16_t x, int16_t y, int16_t ch, int16_t c)
{
    VGLBitmap      *tmp;
    uint8_t          *copy;
    uint8_t           color;
    int16_t           w = 3, h = 12, size;
    int16_t           i;

    if (((ch - 32) >= 0x5f) || (ch < 32))
        return;
    tmp = ch2bmap(alphas[ch - 32], w, h);
    size = tmp->Xsize * tmp->Ysize;
    copy = malloc(size);
    memcpy(copy, tmp->Bitmap, size);

    for (i = size; i != 0;)
    {
        i--;
        color = copy[i];
        if (color == 10)
        {
            if (c == 2)
                color = 12;
            else if (c == 3)
                color = 14;
        }
        else
        {
            if (color == 12)
            {
                if (c == 1)
                    color = 2;
                else if (c == 2)
                    color = 4;
                else if (c == 3)
                    color = 6;
            }
        }
        copy[i] = color;
    }
    tmp->Bitmap = copy;
    vgaputi(x, y, (uint8_t *) & tmp, w, h);
    VGLBitmapDestroy(tmp);
}

void
vgatitle(void)
{
    VGLBitmap      *tmp = NULL;

    vgageti(0, 0, (uint8_t *) & tmp, 80, 200);
    gettitle(tmp->Bitmap);
    vgaputi(0, 0, (uint8_t *) & tmp, 80, 200);
    VGLBitmapDestroy(tmp);
}

void
gretrace(void)
{
    FIXME("gretrace called");
}

void
savescreen(void)
{
    FILE           *f;
    int             i;

    f = fopen("screen.saw", "w");

    for (i = 0; i < (VGLDisplay->Xsize * VGLDisplay->Ysize); i++)
        fputc(VGLDisplay->Bitmap[i], f);
    fclose(f);
}

/*
 * Depreciated functions, necessary only to avoid "Undefined symbol:..."
 * compiler errors.
 */

void
cgainit(void)
{
}
void
cgaclear(void)
{
}
void
cgatitle(void)
{
}
void
cgawrite(int16_t x, int16_t y, int16_t ch, int16_t c)
{
}
void
cgaputim(int16_t x, int16_t y, int16_t ch, int16_t w, int16_t h)
{
}
void
cgageti(int16_t x, int16_t y, uint8_t * p, int16_t w, int16_t h)
{
}
void
cgaputi(int16_t x, int16_t y, uint8_t * p, int16_t w, int16_t h)
{
}
void
cgapal(int16_t pal)
{
}
void
cgainten(int16_t inten)
{
}
int16_t
cgagetpix(int16_t x, int16_t y)
{
    return (0);
}
