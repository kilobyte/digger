#ifndef DEF_H
#define DEF_H

#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__) || defined YOUR_UNIX_LIKE_ARCH_GOING_HERE
#define UNIX
#endif

#include <stdint.h>
#include <stdbool.h>

#define DIR_NONE -1
#define DIR_RIGHT 0
#define DIR_UP 2
#define DIR_LEFT 4
#define DIR_DOWN 6

#define TYPES 5

#define BONUSES 1
#define BAGS 7
#define MONSTERS 6
#define FIREBALLS DIGGERS
#define DIGGERS 2
#define SPRITES (BONUSES+BAGS+MONSTERS+FIREBALLS+DIGGERS)

/* Sprite order is figured out here. By LAST I mean last+1. */

#define FIRSTBONUS 0
#define LASTBONUS (FIRSTBONUS+BONUSES)
#define FIRSTBAG LASTBONUS
#define LASTBAG (FIRSTBAG+BAGS)
#define FIRSTMONSTER LASTBAG
#define LASTMONSTER (FIRSTMONSTER+MONSTERS)
#define FIRSTFIREBALL LASTMONSTER
#define LASTFIREBALL (FIRSTFIREBALL+FIREBALLS)
#define FIRSTDIGGER LASTFIREBALL
#define LASTDIGGER (FIRSTDIGGER+DIGGERS)

#define MWIDTH 15
#define MHEIGHT 10
#define MSIZE MWIDTH*MHEIGHT

#define MAX_REC_BUFFER 262144l
/* I reckon this is enough for about 36 hours of continuous play. */

#define INI_GAME_SETTINGS "Game"
#define INI_GRAPHICS_SETTINGS "Graphics"
#define INI_SOUND_SETTINGS "Sound"
#define INI_KEY_SETTINGS "Keys"

/* using lesser buffer size will break ie. alsa on linux, no reason to use
 * lesser size anyways...
 */
#define DEFAULT_BUFFER 2048

#if defined UNIX
/* While SDL and other X11 related apps could be runned as ordinary user */
#ifdef __FreeBSD__
#include <sys/syslimits.h>
#else /* I donno what is analog of PATH_MAX for Linux :( */
#define PATH_MAX 1024
#endif
#define ININAME strncat(strncpy(malloc(PATH_MAX),getenv("HOME"),PATH_MAX),"/.digger.rc",PATH_MAX)
#else
#define ININAME "DIGGER.INI"
#endif

#ifdef UNIX
#define stricmp(x, y) strcasecmp(x, y)
#define strnicmp(x, y, z) strncasecmp(x, y, z)
#endif

#define DIGGER_VERSION "POK SDL 20110912"

/* Version string:
  First word: your initials if you have changed anything.
  Second word: platform.
  Third word: compilation date in yyyymmdd format. */

#endif /* DEF_H */
