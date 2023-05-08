include config.mk

define COMPILE =
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<
endef

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJS): $(IDIR)/config.h

$(ODIR)/%.o: $(IDIR)/%.c $(IDIR)/%.h ; $(COMPILE)
$(ODIR)/%.o: $(IDIR)/%.c             ; $(COMPILE)

.PHONY: options
options:
	@echo "$(NAME) build options:"
	@echo "CC       = $(CC)"
	@echo "PKGS     = $(PKGS)"
	@echo "SRCS     = $(SRCS)"
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
