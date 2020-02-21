TOP_SRCDIR=../../
SRCDIR=$(TOP_SRCDIR)/src

CC=gcc
CFLAGS=-g3 -O0 `pkg-config glib-2.0 --cflags` -I$(SRCDIR)
LDFLAGS=-lz -lm `pkg-config glib-2.0 --libs`
EXEC=test-btg
SRC= $(wildcard $(SRCDIR)/*.c)
SRC += test-btg.c 
OBJ= $(SRC:.c=.o)

RED=""
BLUE="\033[01;34m"
NC="\033[0m"

all: $(EXEC)

test-btg: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

#main.o: hello.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper test

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC) test.obj

test.obj: $(EXEC)
	@./$(EXEC) > /dev/null
	@grep "# Created" test.obj > /dev/null && sed -i '3d' test.obj || /bin/true

test: test.obj
	@printf "\033[01;32m * \033[0mTesting BTG Reader/Loader...\t\t"
	@$(shell diff test.obj expected.obj > /dev/null)
	@printf "\033[01;34m[\033[0m"
	@if [ $(.SHELLSTATUS) -ne 0 ]; then printf "\033[0;31m !! \033[0m"; else printf "\033[01;32m ok \033[0m"; fi
	@printf "\033[01;34m]\033[0m\n"

