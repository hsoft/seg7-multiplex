MCU ?= attiny45
# attiny45 goes up to 20mhz, but the chips coming out of the factory are apparently clocked at 1mhz
# by default.
F_CPU ?= 1000000UL

AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 
CC = avr-gcc

SUBMODULE_TARGETS = common/README.md

OBJS = $(addprefix src/, main.o seg7multiplex.o)
OBJS += $(addprefix common/, pin.o timer.o intmath.o)

TO_CLEAN = $(OBJS) $(PROGNAME).hex $(PROGNAME).bin

ALL = $(SUBMODULE_TARGETS) $(PROGNAME).hex

include common.mk

CFLAGS = -O3 $(COMMON_CFLAGS) -DF_CPU=$(F_CPU) -mmcu=$(MCU) -c
LDFLAGS = -mmcu=$(MCU)

# Rules

.PHONY: send

send: $(PROGNAME).hex
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$(PROGNAME).hex

$(PROGNAME).bin: $(OBJS)
	$(CC) $(LDFLAGS) $+ -o $@

$(PROGNAME).hex: $(PROGNAME).bin
	avr-objcopy -O ihex -R .eeprom $< $@
