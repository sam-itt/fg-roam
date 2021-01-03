TOP_SRCDIR=../..
SRCDIR=$(TOP_SRCDIR)/src

CC=gcc
CFLAGS=-g3 -O0 -I$(SRCDIR) -I$(TOP_SRCDIR)/lib/cglm/include/
LDFLAGS=-lm
EXEC=test-geoloc
SRC = $(SRCDIR)/geo-location.c
SRC += test-geoloc.c
OBJ = $(SRC:.c=.o)

RED=""
BLUE="\033[01;34m"
NC="\033[0m"

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper test

clean:
	rm -rf $(OBJ)

mrproper: clean
	rm -rf $(EXEC)

test: all
	@printf "\033[01;32m * \033[0mTesting GeoLocation maths..\t\t"
	@$(shell ./test-geoloc > /dev/null)
	@printf "\033[01;34m[\033[0m"
	@if [ $(.SHELLSTATUS) -ne 0 ]; then printf "\033[0;31m !! \033[0m"; else printf "\033[01;32m ok \033[0m"; fi
	@printf "\033[01;34m]\033[0m\n"

