#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include "def.h"

/* Get a line from a buffer. This should be compatible with all 3 text file
   formats: DOS, Unix and Mac. */
static char *sgets(char *buffer, char *s)
{
    int i;
    for (i = 0; buffer[i] && buffer[i] != '\n'; i++)
        s[i] = buffer[i];
    s[i] = 0;
    return buffer + i;
}

/* These are re-implementations of the Windows version of INI filing. */

void WriteINIString(char *section, char *key, char *value, char *filename)
{
    FILE *fp;
    char *buffer, *p, *p0, s1[80], s2[80], s3[80];
    int tl;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fp = fopen(filename, "w");
        if (fp == NULL)
            return;
        fprintf(fp, "[%s]\n", section);
        fprintf(fp, "%s=%s\n\n", key, value);
        fclose(fp);
        return;
    }
    fseek(fp, 0, 2);
    tl = (int)ftell(fp);
    fseek(fp, 0, 0);
    buffer = (char *)malloc(tl + 1);
    if (buffer == NULL)
    {
        fclose(fp);
        return;
    }
    fread(buffer, tl, 1, fp);
    buffer[tl] = 0;
    fclose(fp);
    strcpy(s2, "[");
    strcat(s2, section);
    strcat(s2, "]");
    strcpy(s3, key);
    strcat(s3, "=");
    p = buffer;
    do
    {
        p = sgets(p, s1);
        if (stricmp(s1, s2) == 0)
        {
            do
            {
                p0 = p;
                p = sgets(p, s1);
                if (strnicmp(s1, s3, strlen(s3)) == 0)
                {
                    fp = fopen(filename, "w");
                    if (fp == NULL)
                    {
                        free(buffer);
                        return;
                    }
                    fwrite(buffer, p0 - buffer, 1, fp);
                    fprintf(fp, "%s=%s\n", key, value);
                    fwrite(p, tl - (p - buffer), 1, fp);
                    fclose(fp);
                    free(buffer);
                    return;
                }
            }
            while (s1[0] != 0);
            fp = fopen(filename, "w");
            if (fp == NULL)
            {
                free(buffer);
                return;
            }
            fwrite(buffer, p0 - buffer, 1, fp);
            fprintf(fp, "%s=%s\n", key, value);
            fwrite(p0, tl - (p0 - buffer), 1, fp);
            fclose(fp);
            free(buffer);
            return;
        }
    }
    while (p < buffer + tl);
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        free(buffer);
        return;
    }
    fprintf(fp, "[%s]\n", section);
    fprintf(fp, "%s=%s\n\n", key, value);
    fwrite(buffer, tl, 1, fp);
    fclose(fp);
    free(buffer);
    return;
}

void GetINIString(char *section, char *key, char *def, char *dest,
                  int destsize, char *filename)
{
    FILE *fp;
    char s1[80], s2[80], s3[80];
    /* FIXME: no sense in copying from to the same destination as source,
     * figure out what is really attempted here
     */
    if (dest != def)
        strcpy(dest, def);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return;
    strcpy(s2, "[");
    strcat(s2, section);
    strcat(s2, "]");
    strcpy(s3, key);
    strcat(s3, "=");
    do
    {
        fgets(s1, 80, fp);
        sgets(s1, s1);
        if (stricmp(s1, s2) == 0)
        {
            do
            {
                fgets(s1, 80, fp);
                sgets(s1, s1);
                if (strnicmp(s1, s3, strlen(s3)) == 0)
                {
                    strcpy(dest, s1 + strlen(s3));
                    fclose(fp);
                    return;
                }
            }
            while (s1[0] != 0 && !feof(fp) && !ferror(fp));
        }
    }
    while (!feof(fp) && !ferror(fp));
    fclose(fp);
}

int32_t GetINIInt(char *section, char *key, int32_t def, char *filename)
{
    char buf[80];
    sprintf(buf, "%i", def);
    GetINIString(section, key, buf, buf, 80, filename);
    return atol(buf);
}

void WriteINIInt(char *section, char *key, int32_t value,
                 char *filename)
{
    char buf[80];
    sprintf(buf, "%i", value);
    WriteINIString(section, key, buf, filename);
}

bool GetINIBool(char *section, char *key, bool def, char *filename)
{
    char buf[80];
    sprintf(buf, "%i", def);
    GetINIString(section, key, buf, buf, 80, filename);
    strupr(buf);
    if (buf[0] == 'T')
        return true;
    else
        return atoi(buf);
}

void WriteINIBool(char *section, char *key, bool value,
                  char *filename)
{
    WriteINIString(section, key, value ? "True" : "False", filename);
}
