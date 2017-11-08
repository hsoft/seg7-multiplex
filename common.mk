PROGNAME = seg7multiplex

EXTRACFLAGS ?= 
MAX_DIGITS ?= 4
COMMON_CFLAGS = -Wall $(EXTRACFLAGS) -DMAX_DIGITS=$(MAX_DIGITS) 

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

