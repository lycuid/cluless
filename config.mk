NAME=cluless
VERSION=0.4.12

BUILD=.build
I_DIR=src
O_DIR=$(BUILD)/cache
BIN=$(BUILD)/bin/$(NAME)

PREFIX=/usr/local
BINPREFIX=$(PREFIX)/bin
MANPREFIX=$(PREFIX)/man/man1

O_FILES=$(O_DIR)/$(NAME).o                      \
        $(O_DIR)/$(NAME)/bindings.o             \
        $(O_DIR)/$(NAME)/core.o                 \
        $(O_DIR)/$(NAME)/core/client.o          \
        $(O_DIR)/$(NAME)/core/logging.o         \
        $(O_DIR)/$(NAME)/core/monitor.o         \
        $(O_DIR)/$(NAME)/core/workspace.o       \
        $(O_DIR)/$(NAME)/ewmh.o                 \
        $(O_DIR)/$(NAME)/ewmh/docks.o           \
        $(O_DIR)/$(NAME)/layout.o               \
        $(O_DIR)/$(NAME)/layout/full.o          \
        $(O_DIR)/$(NAME)/layout/tall.o          \
        $(O_DIR)/$(NAME)/misc/companion.o       \
        $(O_DIR)/$(NAME)/misc/magnify.o         \
        $(O_DIR)/$(NAME)/misc/scratchpad.o      \
        $(O_DIR)/$(NAME)/misc/window_rule.o

PKGS=x11
DEFINE=-D_GNU_SOURCE -DNAME='"$(NAME)"' -DVERSION='"$(VERSION)"'
FLAGS=-Wall -Wextra -Wvla -pedantic -I$(I_DIR) -ggdb -O3
