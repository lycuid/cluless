include config.mk

$(BIN): $(NAME).c $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $(NAME).c $(OBJS)

$(OBJS): config.h

$(BUILDDIR)/%.o: %.c %.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

.PHONY: fmt
fmt:
	clang-format -i $(NAME).c $(SRCFILES)
	clang-format -i $(SRCFILES:.c=.h)

install: $(BIN)
	strip --strip-all $(BIN)
	cp -f $(BIN) $(BINPREFIX)/$(NAME)
	chmod 755 $(BINPREFIX)/$(NAME)

.PHONY: uninstall
uninstall:
	$(RM) $(BINPREFIX)/$(NAME)
