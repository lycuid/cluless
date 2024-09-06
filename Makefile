include config.mk

override CFLAGS+= $(FLAGS) $(DEFINE) $(shell pkg-config --cflags $(PKGS))
LDFLAGS:=$(shell pkg-config --libs $(PKGS))

all: $(BIN)

$(BIN): $(O_FILES) ; @mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(O_FILES): $(I_DIR)/config.h

$(O_DIR)/%.o: $(I_DIR)/%.c $(I_DIR)/%.h ; @mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(O_DIR)/%.o: $(I_DIR)/%.c ; @mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: options
options:
	@echo "$(NAME) build options:"
	@echo "CC       = $(CC)"
	@echo "PKGS     = $(PKGS)"
	@echo "O_FILES  = $(O_FILES)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "----------------------------------"

.PHONY: install
install: options $(BIN)
	strip --strip-all $(BIN)
	cp -f $(BIN) $(DESTDIR)$(BINPREFIX)/$(NAME)
	sed -e 's/APPNAME/$(NAME)/g' -e 's/APPVERSION/$(VERSION)/g' $(NAME).1.tmpl \
		| gzip > $(DESTDIR)$(MANPREFIX)/$(NAME).1.gz
	chmod 755 $(DESTDIR)$(BINPREFIX)/$(NAME)
	chmod 644 $(DESTDIR)$(MANPREFIX)/$(NAME).1.gz


.PHONY: uninstall
uninstall:
	mkdir -p $(BINPREFIX)
	$(RM) $(BINPREFIX)/$(NAME)

# misc.
.PHONY: clean fmt loc
clean: ; rm -rf $(BUILD)
fmt: ; @git ls-files | grep -E '\.[ch]$$' | xargs clang-format -i
loc: ; @git ls-files | grep -E '\.[ch]$$' | xargs wc -l
compile_flags: ; @echo $(CFLAGS) | tr ' ' '\n' > compile_flags.txt
