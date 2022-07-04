NAME=cluless
VERSION=0.4.1
BUILD=.build
IDIR=src
ODIR=$(BUILD)/cache
BIN=$(BUILD)/bin/$(NAME)
PREFIX=/usr/local
BINPREFIX=$(PREFIX)/bin

SRCFILES=$(IDIR)/$(NAME).c                 \
         $(IDIR)/$(NAME)/core.c            \
         $(IDIR)/$(NAME)/core/monitor.c    \
         $(IDIR)/$(NAME)/core/workspace.c  \
         $(IDIR)/$(NAME)/core/client.c     \
         $(IDIR)/$(NAME)/ewmh.c            \
         $(IDIR)/$(NAME)/ewmh/docks.c      \
         $(IDIR)/$(NAME)/layout.c          \
         $(IDIR)/$(NAME)/layout/tall.c     \
         $(IDIR)/$(NAME)/scratchpad.c      \
         $(IDIR)/$(NAME)/bindings.c        \
         $(IDIR)/$(NAME)/window_rule.c

OBJS=$(SRCFILES:$(IDIR)/%.c=$(ODIR)/%.o)
PKGS=x11
DEFINE=-D_GNU_SOURCE -DNAME='"$(NAME)"' -DVERSION='"$(VERSION)"'
FLAGS=-Wall -Wextra -pedantic -I$(IDIR) -ggdb -O3
override CFLAGS+=$(FLAGS) $(DEFINE) $(shell pkg-config --cflags $(PKGS))
override LDFLAGS+=$(shell pkg-config --libs $(PKGS))

# Link.
$(BIN): $(OBJS)
	mkdir -p $(@D) && $(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $(OBJS)

$(OBJS): $(IDIR)/config.h $(IDIR)/preprocs.h

# Build.
$(ODIR)/%.o: $(IDIR)/%.c $(IDIR)/%.h $(@D)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c -o $@ $<
$(ODIR)/%.o: $(IDIR)/%.c $(@D)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c -o $@ $<

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
clean: ; rm -rf $(BUILD)
fmt: ; @git ls-files | egrep '\.[ch]$$' | xargs clang-format -i
loc: ; @git ls-files | egrep '\.[ch]$$' | xargs wc -l
compile_flags: ; @echo $(CFLAGS) | tr ' ' '\n' > compile_flags.txt
