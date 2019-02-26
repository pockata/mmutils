include config.mk

HDR = util.h
SRC =	\
	mattr.c \
	pfm.c \
	lsm.c

OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1)

.POSIX:
.SUFFIXES: .1 .1.gz

all: binutils

binutils: $(BIN)

$(OBJ): $(HDR) util.o

.o:
	@echo "LD $@"
	@$(LD) $< util.o -o $@ $(LDFLAGS) $(LIBS)

.c.o:
	@echo "CC $<"
	@$(CC) -c $< -o $@ $(CFLAGS)

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin/
	cp -f $(MAN) $(DESTDIR)$(PREFIX)/share/man/man1/

uninstall:
	@echo "uninstalling binaries"
	@for util in $(BIN); do \
		rm -f $(DESTDIR)$(PREFIX)/bin/$$util; \
	done
	@for page in $(MAN); do\
		rm -f $(DESTDIR)$(PREFIX)/share/man/man1/$$page; \
	done
clean :
	rm -f $(OBJ) $(BIN) util.o
