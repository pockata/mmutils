include config.mk

HDR = util.h
SRC =	\
	mattr.c \
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
	@$(LD) $< util.o -o $@ $(LDFLAGS)

.c.o:
	@echo "CC $<"
	@$(CC) -c $< -o $@ $(CFLAGS)

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin/
	#cd man; $(MAKE) install

uninstall:
	@echo "uninstalling binaries"
	@for util in $(BIN); do \
		rm -f $(DESTDIR)$(PREFIX)/bin/$$util; \
	done
	#cd man; $(MAKE) uninstall

clean :
	rm -f $(OBJ) $(BIN) util.o

