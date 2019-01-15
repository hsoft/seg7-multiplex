PROGNAME = drive
MCU ?= attiny45
F_CPU ?= 1000000UL

AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 
CC = avr-gcc

OBJS = main.o
OBJS += $(addprefix ../common/, pin.o timer.o intmath.o)

TO_CLEAN = $(OBJS) $(PROGNAME).hex $(PROGNAME).bin

CFLAGS = -O3 -Wall -DF_CPU=$(F_CPU) -mmcu=$(MCU) -c
LDFLAGS = -mmcu=$(MCU)

# Rules

.PHONY: all send

all: $(PROGNAME).hex

send: $(PROGNAME).hex
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$(PROGNAME).hex

$(PROGNAME).bin: $(OBJS)
	$(CC) $(LDFLAGS) $+ -o $@

$(PROGNAME).hex: $(PROGNAME).bin
	avr-objcopy -O ihex -R .eeprom $< $@
