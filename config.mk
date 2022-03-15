NAME=wm
VERSION=0.1.0
BUILDDIR=.cache
BIN=$(BUILDDIR)/$(NAME)
PREFIX=/usr/local
BINPREFIX=$(PREFIX)/bin

SRCDIRS=include include/layouts include/ewmh
SRCFILES=$(wildcard $(SRCDIRS:%=%/*.c))
OBJS=$(SRCFILES:%.c=$(BUILDDIR)/%.o)
PKGS=x11

DEFINE=-D_POSIX_C_SOURCE=200809 -DNAME='"$(NAME)"' -DVERSION='"$(VERSION)"'
FLAGS=-Wall -Wextra -pedantic -std=c99 -I. -ggdb -O3
override CFLAGS+=$(FLAGS) $(DEFINE) $(shell pkg-config --cflags $(PKGS))
override LDFLAGS+=$(shell pkg-config --libs $(PKGS))
