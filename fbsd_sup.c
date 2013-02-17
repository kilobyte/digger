#include <ctype.h>
#include <stdlib.h>
#include "def.h"

void strupr(char *str)
{
    while (*str != 0)
    {
        *str = toupper(*str);
        str++;
    }
}
