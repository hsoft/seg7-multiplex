PROGNAME = seg7multiplex

EXTRACFLAGS ?= 
COMMON_CFLAGS = -Wall $(EXTRACFLAGS)

.PHONY: clean

# Patterns
%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Rules

all: $(ALL)
	@echo "Done!"

$(SUBMODULE_TARGETS):
	git submodule init
	git submodule update

clean:
	rm -f $(TO_CLEAN)

