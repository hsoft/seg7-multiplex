OBJS = main.o
OBJS += $(addprefix ../src/, seg7multiplex.o)
OBJS += $(addprefix ../common/, intmath.o)

TO_CLEAN = $(OBJS) $(PROGNAME)

SUBMODULE_TARGETS = ../common/README.md

ALL = $(SUBMODULE_TARGETS) $(PROGNAME)

include ../common.mk

CFLAGS = -I. -Iicemu/src -DSIMULATION $(COMMON_CFLAGS) -c
LDFLAGS = -Licemu -licemu `pkg-config --libs --static ncurses`

# Rules
$(PROGNAME): $(OBJS)
	$(MAKE) -C icemu clean all
	$(CC) $+ -o $@ $(LDFLAGS)

