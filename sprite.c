#include <stdlib.h>
#include "def.h"
#include "sprite.h"
#include "hardware.h"

static bool sprrdrwf[SPRITES + 1];
static bool sprrecf[SPRITES + 1];
static bool sprenf[SPRITES];
static int16_t sprch[SPRITES + 1];
static uint8_t *sprmov[SPRITES];
static int16_t sprx[SPRITES + 1];
static int16_t spry[SPRITES + 1];
static int16_t sprwid[SPRITES + 1];
static int16_t sprhei[SPRITES + 1];
static int16_t sprbwid[SPRITES];
static int16_t sprbhei[SPRITES];
static int16_t sprnch[SPRITES];
static int16_t sprnwid[SPRITES];
static int16_t sprnhei[SPRITES];
static int16_t sprnbwid[SPRITES];
static int16_t sprnbhei[SPRITES];

static void clearrdrwf(void);
static void clearrecf(void);
static void setrdrwflgs(int16_t n);
static bool collide(int16_t bx, int16_t si);
static bool bcollide(int16_t bx, int16_t si);
static void putims(void);
static void putis(void);
static void bcollides(int bx);

void (*ginit)(void) = vgainit;
void (*gclear)(void) = vgaclear;
void (*gpal)(int16_t pal) = vgapal;
void (*ginten)(int16_t inten) = vgainten;
void (*gputi)(int16_t x, int16_t y, uint8_t *p, int16_t w, int16_t h) = vgaputi;
void (*ggeti)(int16_t x, int16_t y, uint8_t *p, int16_t w, int16_t h) = vgageti;
void (*gputim)(int16_t x, int16_t y, int16_t ch, int16_t w, int16_t h) = vgaputim;
int16_t (*ggetpix)(int16_t x, int16_t y) = vgagetpix;
void (*gtitle)(void) = vgatitle;
void (*gwrite)(int16_t x, int16_t y, int16_t ch, int16_t c) = vgawrite;

void setretr(bool f)
{
}

void createspr(int16_t n, int16_t ch, uint8_t *mov, int16_t wid, int16_t hei, int16_t bwid,
               int16_t bhei)
{
    sprnch[n] = sprch[n] = ch;
    sprmov[n] = mov;
    sprnwid[n] = sprwid[n] = wid;
    sprnhei[n] = sprhei[n] = hei;
    sprnbwid[n] = sprbwid[n] = bwid;
    sprnbhei[n] = sprbhei[n] = bhei;
    sprenf[n] = false;
}

void movedrawspr(int16_t n, int16_t x, int16_t y)
{
    sprx[n] = x & -4;
    spry[n] = y;
    sprch[n] = sprnch[n];
    sprwid[n] = sprnwid[n];
    sprhei[n] = sprnhei[n];
    sprbwid[n] = sprnbwid[n];
    sprbhei[n] = sprnbhei[n];
    clearrdrwf();
    setrdrwflgs(n);
    putis();
    ggeti(sprx[n], spry[n], sprmov[n], sprwid[n], sprhei[n]);
    sprenf[n] = true;
    sprrdrwf[n] = true;
    putims();
}

void erasespr(int16_t n)
{
    if (!sprenf[n])
        return;
    gputi(sprx[n], spry[n], sprmov[n], sprwid[n], sprhei[n]);
    sprenf[n] = false;
    clearrdrwf();
    setrdrwflgs(n);
    putims();
}

void drawspr(int16_t n, int16_t x, int16_t y)
{
    int16_t t1, t2, t3, t4;
    x &= -4;
    clearrdrwf();
    setrdrwflgs(n);
    t1 = sprx[n];
    t2 = spry[n];
    t3 = sprwid[n];
    t4 = sprhei[n];
    sprx[n] = x;
    spry[n] = y;
    sprwid[n] = sprnwid[n];
    sprhei[n] = sprnhei[n];
    clearrecf();
    setrdrwflgs(n);
    sprhei[n] = t4;
    sprwid[n] = t3;
    spry[n] = t2;
    sprx[n] = t1;
    sprrdrwf[n] = true;
    putis();
    sprenf[n] = true;
    sprx[n] = x;
    spry[n] = y;
    sprch[n] = sprnch[n];
    sprwid[n] = sprnwid[n];
    sprhei[n] = sprnhei[n];
    sprbwid[n] = sprnbwid[n];
    sprbhei[n] = sprnbhei[n];
    ggeti(sprx[n], spry[n], sprmov[n], sprwid[n], sprhei[n]);
    putims();
    bcollides(n);
}

void initspr(int16_t n, int16_t ch, int16_t wid, int16_t hei, int16_t bwid, int16_t bhei)
{
    sprnch[n] = ch;
    sprnwid[n] = wid;
    sprnhei[n] = hei;
    sprnbwid[n] = bwid;
    sprnbhei[n] = bhei;
}

void initmiscspr(int16_t x, int16_t y, int16_t wid, int16_t hei)
{
    sprx[SPRITES] = x;
    spry[SPRITES] = y;
    sprwid[SPRITES] = wid;
    sprhei[SPRITES] = hei;
    clearrdrwf();
    setrdrwflgs(SPRITES);
    putis();
}

void getis(void)
{
    int16_t i;
    for (i = 0; i < SPRITES; i++)
        if (sprrdrwf[i])
            ggeti(sprx[i], spry[i], sprmov[i], sprwid[i], sprhei[i]);
    putims();
}

void drawmiscspr(int16_t x, int16_t y, int16_t ch, int16_t wid, int16_t hei)
{
    sprx[SPRITES] = x & -4;
    spry[SPRITES] = y;
    sprch[SPRITES] = ch;
    sprwid[SPRITES] = wid;
    sprhei[SPRITES] = hei;
    gputim(sprx[SPRITES], spry[SPRITES], sprch[SPRITES], sprwid[SPRITES],
           sprhei[SPRITES]);
}

static void clearrdrwf(void)
{
    int16_t i;
    clearrecf();
    for (i = 0; i < SPRITES + 1; i++)
        sprrdrwf[i] = false;
}

static void clearrecf(void)
{
    int16_t i;
    for (i = 0; i < SPRITES + 1; i++)
        sprrecf[i] = false;
}

static void setrdrwflgs(int16_t n)
{
    int16_t i;
    if (!sprrecf[n])
    {
        sprrecf[n] = true;
        for (i = 0; i < SPRITES; i++)
            if (sprenf[i] && i != n)
            {
                if (collide(i, n))
                {
                    sprrdrwf[i] = true;
                    setrdrwflgs(i);
                }
            }
    }
}

static bool collide(int16_t bx, int16_t si)
{
    if (sprx[bx] >= sprx[si])
    {
        if (sprx[bx] > (sprwid[si] << 2) + sprx[si] - 1)
            return false;
    }
    else if (sprx[si] > (sprwid[bx] << 2) + sprx[bx] - 1)
        return false;
    if (spry[bx] >= spry[si])
    {
        if (spry[bx] <= sprhei[si] + spry[si] - 1)
            return true;
        return false;
    }
    if (spry[si] <= sprhei[bx] + spry[bx] - 1)
        return true;
    return false;
}

static bool bcollide(int16_t bx, int16_t si)
{
    if (sprx[bx] >= sprx[si])
    {
        if (sprx[bx] + sprbwid[bx] > (sprwid[si] << 2) + sprx[si] - sprbwid[si] - 1)
            return false;
    }
    else if (sprx[si] + sprbwid[si] > (sprwid[bx] << 2) + sprx[bx] - sprbwid[bx] - 1)
        return false;
    if (spry[bx] >= spry[si])
    {
        if (spry[bx] + sprbhei[bx] <= sprhei[si] + spry[si] - sprbhei[si] - 1)
            return true;
        return false;
    }
    if (spry[si] + sprbhei[si] <= sprhei[bx] + spry[bx] - sprbhei[bx] - 1)
        return true;
    return false;
}

static void putims(void)
{
    int i;
    for (i = 0; i < SPRITES; i++)
        if (sprrdrwf[i])
            gputim(sprx[i], spry[i], sprch[i], sprwid[i], sprhei[i]);
}

static void putis(void)
{
    int i;
    for (i = 0; i < SPRITES; i++)
        if (sprrdrwf[i])
            gputi(sprx[i], spry[i], sprmov[i], sprwid[i], sprhei[i]);
}

int first[TYPES], coll[SPRITES];
static const int firstt[TYPES] =
{FIRSTBONUS, FIRSTBAG, FIRSTMONSTER, FIRSTFIREBALL, FIRSTDIGGER};
static const int lastt[TYPES] =
{LASTBONUS, LASTBAG, LASTMONSTER, LASTFIREBALL, LASTDIGGER};

static void bcollides(int spr)
{
    int spc, next, i;
    for (next = 0; next < TYPES; next++)
        first[next] = -1;
    for (next = 0; next < SPRITES; next++)
        coll[next] = -1;
    for (i = 0; i < TYPES; i++)
    {
        next = -1;
        for (spc = firstt[i]; spc < lastt[i]; spc++)
            if (sprenf[spc] && spc != spr)
                if (bcollide(spr, spc))
                {
                    if (next == -1)
                        first[i] = next = spc;
                    else
                        coll[next = (coll[next] = spc)] = -1;
                }
    }
}
