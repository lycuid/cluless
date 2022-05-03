NAME=wm
VERSION=0.2.7
ODIR=build
IDIR=src
BIN=$(ODIR)/$(NAME)
PREFIX=/usr/local
BINPREFIX=$(PREFIX)/bin

SRCDIRS=$(IDIR)/include              \
        $(IDIR)/include/core         \
        $(IDIR)/include/layout       \
        $(IDIR)/include/ewmh
SRCFILES=$(wildcard $(SRCDIRS:%=%/*.c))
OBJS=$(ODIR)/$(NAME).o $(SRCFILES:$(IDIR)/%.c=$(ODIR)/%.o)

PKGS=x11
DEFINE=-D_GNU_SOURCE -DNAME='"$(NAME)"' -DVERSION='"$(VERSION)"'
FLAGS=-Wall -Wextra -pedantic -I$(IDIR) -ggdb -O3
override CFLAGS+=$(FLAGS) $(DEFINE) $(shell pkg-config --cflags $(PKGS))
override LDFLAGS+=$(shell pkg-config --libs $(PKGS))

define compile-src =
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<
endef

# Linking.
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $(OBJS)

# Buidling.
$(ODIR)/%.o: $(IDIR)/%.c $(IDIR)/%.h ; $(compile-src)
$(ODIR)/%.o: $(IDIR)/%.c             ; $(compile-src)

$(OBJS): $(IDIR)/config.h $(IDIR)/preprocs.h

# Install/uninstall.
.PHONY: install
install: $(BIN)
	strip --strip-all $(BIN)
	cp -f $(BIN) $(BINPREFIX)/$(NAME)
	chmod 755 $(BINPREFIX)/$(NAME)

.PHONY: uninstall
uninstall:
	$(RM) $(BINPREFIX)/$(NAME)

# misc.
.PHONY: clean fmt loc
clean: ; rm -rf $(ODIR)
fmt: ; @git ls-files | egrep '\.[ch]$$' | xargs clang-format -i
loc: ; @git ls-files | egrep '\.[ch]$$' | xargs wc -l
compile_flags: compile_flags.txt
	echo $(CFLAGS) | xargs -n1 > compile_flags.txt
compile_flags.txt:
	touch $@
