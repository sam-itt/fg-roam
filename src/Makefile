TOP_SRCDIR=..
SRCDIR=$(TOP_SRCDIR)/src
FG_IO=$(TOP_SRCDIR)/lib/fg-io


CC=gcc
CFLAGS=-g3 -O0 `pkg-config glib-2.0 sdl2 SDL2_image --cflags` \
	   -I$(SRCDIR) \
	   -I$(TOP_SRCDIR)/lib/cglm/include/ \
	   -I$(FG_IO)/flightgear-connector \
	   -I$(FG_IO)/fg-tape
LDFLAGS=-lz -lm `pkg-config glib-2.0 sdl2 SDL2_image --libs` -lGL
EXEC=view-gl
SRC= $(wildcard $(SRCDIR)/*.c)
SRC+= $(filter-out $(FG_IO)/flightgear-connector/fg-connector-test.c, $(wildcard $(FG_IO)/flightgear-connector/*.c))
SRC+= $(filter-out $(FG_IO)/fg-tape/fg-tape-reader.c, $(wildcard $(FG_IO)/fg-tape/*.c))

OBJ= $(SRC:.c=.o)

all: $(EXEC)

view-gl: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper test

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

