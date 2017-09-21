OBJS = main.o
OBJS += $(addprefix ../src/, seg7multiplex.o)
OBJS += $(addprefix ../common/, sim.o sim_pin.o sim_timer.o)
PROGNAME = seg7multiplex

PYTHON ?= python3
PYTHONVERSION := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_python_version(), end='')")
INCS := $(shell pkg-config --cflags python-$(PYTHONVERSION))
LIBS := $(shell pkg-config --libs python-$(PYTHONVERSION))

EXTRACFLAGS ?= 
CFLAGS = $(INCS) -DSIMULATION -Wall $(EXTRACFLAGS) -c
LDFLAGS = $(LIBS)

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

