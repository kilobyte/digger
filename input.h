void detectjoy(void);
bool teststart(void);
void readdirect(int n);
int16_t getdirect(int n);
void checkkeyb(void);
void flushkeybuf(void);
void findkey(int kn);
void clearfire(int n);

extern bool firepflag, fire2pflag, escape;
extern int8_t keypressed;
extern int16_t akeypressed;

extern int keycodes[17][5];
extern bool krdf[17];
extern bool pausef;
