include config.mk

$(BIN): $(OBJS)
	mkdir -p $(@D) && $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJS): $(IDIR)/config.h

$(ODIR)/%.o: $(IDIR)/%.c $(IDIR)/%.h
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c -o $@ $<
$(ODIR)/%.o: $(IDIR)/%.c
	mkdir -p $(@D) && $(CC) $(CFLAGS) -c -o $@ $<

.PHONY: options
options:
	@echo "$(NAME) build options:"
	@echo "CC       = $(CC)"
	@echo "PKGS     = $(PKGS)"
	@echo "SRC      = $(SRC)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "----------------------------------"

.PHONY: install
install: options $(BIN)
	strip --strip-all $(BIN)
	cp -f $(BIN) $(BINPREFIX)/$(NAME)
	chmod 755 $(BINPREFIX)/$(NAME)

.PHONY: uninstall
uninstall:
	mkdir -p $(BINPREFIX)
	$(RM) $(BINPREFIX)/$(NAME)

# misc.
.PHONY: clean fmt loc
clean: ; rm -rf $(BUILD)
fmt: ; @git ls-files | egrep '\.[ch]$$' | xargs clang-format -i
loc: ; @git ls-files | egrep '\.[ch]$$' | xargs wc -l
compile_flags: ; @echo $(CFLAGS) | tr ' ' '\n' > compile_flags.txt
