#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "def.h"
#include "scores.h"
#include "main.h"
#include "drawing.h"
#include "hardware.h"
#include "sound.h"
#include "sprite.h"
#include "input.h"
#include "digger.h"
#include "record.h"

static struct scdat
{
    int32_t score, nextbs;
} scdat[DIGGERS];

static char highbuf[10];

static int32_t scorehigh[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char scoreinit[11][4];

int32_t scoret = 0;

static char hsbuf[36];

static char scorebuf[512];

uint16_t bonusscore = 20000;

static void readscores(void);
static void writescores(void);
static void savescores(void);
static void getinitials(void);
static void flashywait(int16_t n);
static int16_t getinitial(int16_t x, int16_t y);
static void shufflehigh(void);
static void writenum(int32_t n, int16_t x, int16_t y, int16_t w, int16_t c);
static void numtostring(char *p, int32_t n);

#if defined UNIX

#define SFNAME strncat(strncpy(malloc(PATH_MAX),getenv("HOME"),PATH_MAX),"/.digger.sco",PATH_MAX)

#else

#define SFNAME "DIGGER.SCO"

#endif

#ifdef INTDRF
int32_t getscore0(void)
{
    return scdat[0].score;
}
#endif

static void readscores(void)
{
    FILE *in;
    scorebuf[0] = 0;
    if (!levfflag)
    {
        if ((in = fopen(SFNAME, "rb")) != NULL)
        {
            fread(scorebuf, 512, 1, in);
            fclose(in);
        }
    }
    else if ((in = fopen(levfname, "rb")) != NULL)
    {
        fseek(in, 1202, 0);
        fread(scorebuf, 512, 1, in);
        fclose(in);
    }
}

static void writescores(void)
{
    FILE *out;
    if (!levfflag)
    {
        if ((out = fopen(SFNAME, "wb")) != NULL)
        {
            fwrite(scorebuf, 512, 1, out);
            fclose(out);
        }
    }
    else if ((out = fopen(levfname, "r+b")) != NULL)
    {
        fseek(out, 1202, 0);
        fwrite(scorebuf, 512, 1, out);
        fclose(out);
    }
}

void initscores(void)
{
    int i;
    for (i = 0; i < diggers; i++)
        addscore(i, 0);
}

void loadscores(void)
{
    int16_t p = 0, i, x;
    readscores();
    if (gauntlet)
        p = 111;
    if (diggers == 2)
        p += 222;
    if (scorebuf[p++] != 's')
        for (i = 0; i < 11; i++)
        {
            scorehigh[i + 1] = 0;
            strcpy(scoreinit[i], "...");
        }
    else
        for (i = 1; i < 11; i++)
        {
            for (x = 0; x < 3; x++)
                scoreinit[i][x] = scorebuf[p++];
            p += 2;
            for (x = 0; x < 6; x++)
                highbuf[x] = scorebuf[p++];
            scorehigh[i + 1] = atol(highbuf);
        }
}

void zeroscores(void)
{
    scdat[0].score = scdat[1].score = 0;
    scdat[0].nextbs = scdat[1].nextbs = bonusscore;
    scoret = 0;
}

void writecurscore(int col)
{
    if (curplayer == 0)
        writenum(scdat[0].score, 0, 0, 6, col);
    else if (scdat[1].score < 100000l)
        writenum(scdat[1].score, 236, 0, 6, col);
    else
        writenum(scdat[1].score, 248, 0, 6, col);
}

void drawscores(void)
{
    writenum(scdat[0].score, 0, 0, 6, 3);
    if (nplayers == 2 || diggers == 2)
    {
        if (scdat[1].score < 100000l)
            writenum(scdat[1].score, 236, 0, 6, 3);
        else
            writenum(scdat[1].score, 248, 0, 6, 3);
    }
}

void addscore(int n, int16_t score)
{
    scdat[n].score += score;
    if (scdat[n].score > 999999l)
        scdat[n].score = 0;
    if (n == 0)
        writenum(scdat[n].score, 0, 0, 6, 1);
    else if (scdat[n].score < 100000l)
        writenum(scdat[n].score, 236, 0, 6, 1);
    else
        writenum(scdat[n].score, 248, 0, 6, 1);
    if (scdat[n].score >= scdat[n].nextbs + n) /* +n to reproduce original bug */
    {
        if (getlives(n) < 5)
        {
            if (gauntlet)
                cgtime += 17897715l; /* 15 second time bonus instead of the life */
            else
                addlife(n);
            drawlives();
        }
        scdat[n].nextbs += bonusscore;
    }
    incpenalty();
    incpenalty();
    incpenalty();
}

void endofgame(void)
{
    int16_t i;
    bool initflag = false;
    for (i = 0; i < diggers; i++)
        addscore(i, 0);
    if (playing || !drfvalid)
        return;
    if (gauntlet)
    {
        cleartopline();
        outtext("TIME UP", 120, 0, 3);
        for (i = 0; i < 50 && !escape; i++)
            newframe();
        outtext("       ", 120, 0, 3);
    }
    for (i = curplayer; i < curplayer + diggers; i++)
    {
        scoret = scdat[i].score;
        if (scoret > scorehigh[11])
        {
            gclear();
            drawscores();
            strcpy(pldispbuf, "PLAYER ");
            if (i == 0)
                strcat(pldispbuf, "1");
            else
                strcat(pldispbuf, "2");
            outtext(pldispbuf, 108, 0, 2);
            outtext(" NEW HIGH SCORE ", 64, 40, 2);
            getinitials();
            shufflehigh();
            savescores();
            initflag = true;
        }
    }
    if (!initflag && !gauntlet)
    {
        cleartopline();
        outtext("GAME OVER", 104, 0, 3);
        for (i = 0; i < 50 && !escape; i++)
            newframe();
        outtext("         ", 104, 0, 3);
    }
}

void showtable(void)
{
    int16_t i, col;
    outtext("HIGH SCORES", 16, 25, 3);
    col = 2;
    for (i = 1; i < 11; i++)
    {
        strcpy(hsbuf, "");
        strcat(hsbuf, scoreinit[i]);
        strcat(hsbuf, "  ");
        numtostring(highbuf, scorehigh[i + 1]);
        strcat(hsbuf, highbuf);
        outtext(hsbuf, 16, 31 + 13 * i, col);
        col = 1;
    }
}

static void savescores(void)
{
    int16_t i, p = 0, j;
    if (gauntlet)
        p = 111;
    if (diggers == 2)
        p += 222;
    strcpy(scorebuf + p, "s");
    for (i = 1; i < 11; i++)
    {
        strcpy(hsbuf, "");
        strcat(hsbuf, scoreinit[i]);
        strcat(hsbuf, "  ");
        numtostring(highbuf, scorehigh[i + 1]);
        strcat(hsbuf, highbuf);
        for (j = 0; j < 11; j++)
            scorebuf[p + j + i * 11 - 10] = hsbuf[j];
    }
    writescores();
}

static void getinitials(void)
{
    int16_t k, i;
    newframe();
    outtext("ENTER YOUR", 100, 70, 3);
    outtext(" INITIALS", 100, 90, 3);
    outtext("_ _ _", 128, 130, 3);
    strcpy(scoreinit[0], "...");
    killsound();
    for (i = 0; i < 3; i++)
    {
        k = 0;
        while (k == 0)
        {
            k = getinitial(i * 24 + 128, 130);
            if (k == 8 || k == 127)
            {
                if (i > 0)
                    i--;
                k = 0;
            }
        }
        if (k != 0)
        {
            gwrite(i * 24 + 128, 130, k, 3);
            scoreinit[0][i] = k;
        }
    }
    for (i = 0; i < 20; i++)
        flashywait(15);
    setupsound();
    gclear();
    gpal(0);
    ginten(0);
    recputinit(scoreinit[0]);
}

static void flashywait(int16_t n)
{
    int16_t i, cx, p = 0;
    for (i = 0; i < (n << 1); i++)
        for (cx = 0; cx < volume; cx++)
            gpal(p = 1 - p);
}

static int16_t getinitial(int16_t x, int16_t y)
{
    int16_t i;
    gwrite(x, y, '_', 3);
    do
    {
        for (i = 0; i < 40; i++)
        {
            if (kbhit())
            {
                int16_t key = getkey();
                if (!isalnum(key))
                    continue;
                return key;
            }
            flashywait(15);
        }
        for (i = 0; i < 40; i++)
        {
            if (kbhit())
            {
                gwrite(x, y, '_', 3);
                return getkey();
            }
            flashywait(15);
        }
    }
    while (1);
}

static void shufflehigh(void)
{
    int16_t i, j;
    for (j = 10; j > 1; j--)
        if (scoret < scorehigh[j])
            break;
    for (i = 10; i > j; i--)
    {
        scorehigh[i + 1] = scorehigh[i];
        strcpy(scoreinit[i], scoreinit[i - 1]);
    }
    scorehigh[j + 1] = scoret;
    strcpy(scoreinit[j], scoreinit[0]);
}

void scorekill(int n)
{
    addscore(n, 250);
}

void scorekill2(void)
{
    addscore(0, 125);
    addscore(1, 125);
}

void scoreemerald(int n)
{
    addscore(n, 25);
}

void scoreoctave(int n)
{
    addscore(n, 250);
}

void scoregold(int n)
{
    addscore(n, 500);
}

void scorebonus(int n)
{
    addscore(n, 1000);
}

void scoreeatm(int n, int msc)
{
    addscore(n, msc * 200);
}

static void writenum(int32_t n, int16_t x, int16_t y, int16_t w, int16_t c)
{
    int16_t d, xp = (w - 1) * 12 + x;
    while (w > 0)
    {
        d = (int16_t)(n % 10);
        if (w > 1 || d > 0)
            gwrite(xp, y, d + '0', c);
        n /= 10;
        w--;
        xp -= 12;
    }
}

static void numtostring(char *p, int32_t n)
{
    int x;
    for (x = 0; x < 6; x++)
    {
        p[5 - x] = (int8_t)(n % 10l) + '0';
        n /= 10l;
        if (n == 0l)
        {
            x++;
            break;
        }
    }
    for (; x < 6; x++)
        p[5 - x] = ' ';
    p[6] = 0;
}
