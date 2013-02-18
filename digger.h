void dodigger(void);
void erasediggers(void);
void killfire(int n);
void erasebonus(void);
int16_t countem(void);
void makeemfield(void);
void drawemeralds(void);
void initdigger(void);
void drawexplosion(int n);
void updatefire(int n);
void createbonus(void);
int16_t reversedir(int16_t d);
bool hitemerald(int16_t x, int16_t y, int16_t rx, int16_t ry, int16_t dir);
void killdigger(int n, int16_t bp6, int16_t bp8);
bool checkdiggerunderbag(int16_t h, int16_t v);
void killemerald(int16_t bpa, int16_t bpc);
void newframe(void);
int diggerx(int n);
int diggery(int n);
void digresettime(int n);
void sceatm(int n);
bool isalive(void);
bool digalive(int n);
int getlives(int pl);
void addlife(int pl);
void initlives(void);
void declife(int pl);

extern bool bonusvisible, digonscr, bonusmode;
extern uint32_t ftime, curtime, cgtime;

#ifdef INTDRF
extern uint32_t frame;
#endif
