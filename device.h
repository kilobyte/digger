/* Generic sound device header */

#define MIN_SAMP 0
#define MAX_SAMP 0xff

bool setsounddevice(uint16_t samprate, uint16_t bufsize);
bool initsounddevice(void);
void killsounddevice(void);
