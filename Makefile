CC	?= gcc
CFLAGS	+= -O -g -pipe
RCFLAGS = -std=c99 -Wall
OBJS	= main.o digger.o drawing.o sprite.o scores.o record.o sound.o \
	newsnd.o ini.o input.o monster.o bags.o alpha.o vgagrafx.o \
	title_gz.o icon.o sdl_kbd.o sdl_vid.o sdl_timer.o sdl_snd.o

#ARCH	= "MINGW"

ifeq ($(ARCH),"MINGW")
RCFLAGS	+= -mno-cygwin -DMINGW -Dmain=SDL_main -I../zlib -I../SDL-1.1.2/include/SDL
LIBS	+= -mno-cygwin -mwindows -lmingw32 -L../SDL-1.1.2/lib -lSDLmain -lSDL -luser32 -lgdi32 -lwinmm -L../zlib -lz
ESUFFIX	=  .exe
else
RCFLAGS	+= $(shell sdl-config --cflags)
LIBS	+= $(shell sdl-config --libs) -lz
ESUFFIX	=
endif

all: digger$(ESUFFIX)

digger$(ESUFFIX): $(OBJS)
	$(CC) -o digger$(ESUFFIX) $(OBJS) $(LIBS)

$(OBJS): %.o: %.c *.h
	$(CC) -c $(RCFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) digger$(ESUFFIX)
