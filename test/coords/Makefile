TOP_SRCDIR=../..
SRCDIR=$(TOP_SRCDIR)/src

CC=gcc
CFLAGS=-g3 -O0 `pkg-config glib-2.0 --cflags` -I$(SRCDIR) -I$(TOP_SRCDIR)/lib/cglm/include/
LDFLAGS=-lz -lm `pkg-config glib-2.0 --libs`
EXEC=test-coords
SRC = $(SRCDIR)/geodesy.c
SRC += test-coords.c 
OBJ= $(SRC:.c=.o)

RED=""
BLUE="\033[01;34m"
NC="\033[0m"

all: $(EXEC)

test-coords: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper test

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC) 

test: all
	@printf "\033[01;32m * \033[0mTesting Coord maths..\t\t"
	@$(shell ./test-coords)
	@printf "\033[01;34m[\033[0m"
	@if [ $(.SHELLSTATUS) -ne 0 ]; then printf "\033[0;31m !! \033[0m"; else printf "\033[01;32m ok \033[0m"; fi
	@printf "\033[01;34m]\033[0m\n"

