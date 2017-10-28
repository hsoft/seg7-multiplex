OBJS = main.o
OBJS += $(addprefix ../src/, seg7multiplex.o)
OBJS += $(addprefix ../common/, sim_pin.o sim_timer.o intmath.o)
OBJS += $(addprefix icemu/capi/, icemu.o)

TO_CLEAN = $(OBJS) $(PROGNAME)

SUBMODULE_TARGETS = ../common/README.md

ALL = $(SUBMODULE_TARGETS) $(PROGNAME)

include ../common.mk

CFLAGS = -I. -DSIMULATION $(COMMON_CFLAGS) -c
LDFLAGS = -I.

# Rules
$(PROGNAME): $(OBJS)
	$(CC) $(LDFLAGS) $+ -o $@

