NAME=cluless
VERSION=0.4.12
BUILD=.build
IDIR=src
ODIR=$(BUILD)/cache
BIN=$(BUILD)/bin/$(NAME)
PREFIX=/usr/local
BINPREFIX=$(PREFIX)/bin
MANPREFIX=$(PREFIX)/man/man1

SRCS=$(IDIR)/$(NAME).c                      \
     $(IDIR)/$(NAME)/bindings.c             \
     $(IDIR)/$(NAME)/core.c                 \
     $(IDIR)/$(NAME)/core/client.c          \
     $(IDIR)/$(NAME)/core/logging.c         \
     $(IDIR)/$(NAME)/core/monitor.c         \
     $(IDIR)/$(NAME)/core/workspace.c       \
     $(IDIR)/$(NAME)/ewmh.c                 \
     $(IDIR)/$(NAME)/ewmh/docks.c           \
     $(IDIR)/$(NAME)/layout.c               \
     $(IDIR)/$(NAME)/layout/full.c          \
     $(IDIR)/$(NAME)/layout/tall.c          \
     $(IDIR)/$(NAME)/misc/companion.c       \
     $(IDIR)/$(NAME)/misc/magnify.c         \
     $(IDIR)/$(NAME)/misc/scratchpad.c      \
     $(IDIR)/$(NAME)/misc/window_rule.c

OBJS=$(SRCS:$(IDIR)/%.c=$(ODIR)/%.o)
PKGS=x11
DEFINE=-D_GNU_SOURCE -DNAME='"$(NAME)"' -DVERSION='"$(VERSION)"'
FLAGS=-Wall -Wextra -Wvla -pedantic -I$(IDIR) -ggdb -O3
override CFLAGS+=$(FLAGS) $(DEFINE) $(shell pkg-config --cflags $(PKGS))
override LDFLAGS+=$(shell pkg-config --libs $(PKGS))
