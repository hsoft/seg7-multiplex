MCU ?= attiny45
# attiny45 goes up to 20mhz, but the chips coming out of the factory are apparently clocked at 1mhz
# by default.
F_CPU ?= 1000000UL

AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 

OBJS = $(addprefix src/, main.o seg7multiplex.o)
OBJS += $(addprefix common/, pin.o timer.o intmath.o)
PROGNAME = seg7multiplex

CC = avr-gcc
EXTRACFLAGS ?= 
CFLAGS = -O3 -Wall $(EXTRACFLAGS) -DF_CPU=$(F_CPU) -mmcu=$(MCU) -c
LDFLAGS = -mmcu=$(MCU)

SUBMODULE_TARGETS = common/README.md

# Patterns

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Rules

.PHONY: send clean

all: $(SUBMODULE_TARGETS) $(PROGNAME).hex
	@echo Done!

send: $(PROGNAME).hex
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$(PROGNAME).hex

clean:
	rm -f $(OBJS) $(PROGNAME).hex $(PROGNAME).bin

$(SUBMODULE_TARGETS):
	git submodule init
	git submodule update

$(PROGNAME).bin: $(OBJS)
	$(CC) $(LDFLAGS) $+ -o $@

$(PROGNAME).hex: $(PROGNAME).bin
	avr-objcopy -O ihex -R .eeprom $< $@
