/* Generic sound device header */

#define MIN_SAMP 0
#define MAX_SAMP 0xff

extern uint8_t *buffer;
extern uint16_t firsts, last, size;

bool setsounddevice(int base, int irq, int dma, uint16_t samprate, uint16_t bufsize);
bool initsounddevice(void);
void killsounddevice(void);
