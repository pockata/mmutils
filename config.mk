PREFIX = /usr
MANPREFIX = $(PREFIX)/share/man

CC      = cc
LD      = $(CC)

CFLAGS  = -std=c99 -pedantic -Wall -Os
LIBS = -lxcb -lxcb-randr

