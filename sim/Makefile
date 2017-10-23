OBJS = main.o
OBJS += $(addprefix ../src/, seg7multiplex.o)
OBJS += $(addprefix ../common/, sim_pin.o sim_timer.o intmath.o)
OBJS += $(addprefix icemu/capi/, icemu.o)
PROGNAME = seg7multiplex

EXTRACFLAGS ?= 
CFLAGS = -I. -DSIMULATION -Wall $(EXTRACFLAGS) -c
LDFLAGS = -I.

SUBMODULE_TARGETS = ../common/README.md

# Patterns
%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Rules

.PHONY: clean

all: $(SUBMODULE_TARGETS) $(PROGNAME)
	@echo Done!

clean:
	rm -f $(OBJS) $(PROGNAME)

$(SUBMODULE_TARGETS):
	git submodule init
	git submodule update

$(PROGNAME): $(OBJS)
	$(CC) $(LDFLAGS) $+ -o $@

