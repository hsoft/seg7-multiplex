PROGNAME = seg7multiplex
MCU ?= attiny45
AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 

LDFLAGS = -mmcu=$(MCU)

# Rules

.PHONY: send all clean

all: $(PROGNAME).hex
	@echo Done!

send: $(PROGNAME).hex
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$(PROGNAME).hex

$(PROGNAME).bin: $(PROGNAME).S
	avr-gcc -mmcu=$(MCU) -o $@ $< -nostdlib

$(PROGNAME).hex: $(PROGNAME).bin
	avr-objcopy -O ihex -R .eeprom $< $@

simulation: sim.c $(PROGNAME).bin
	$(CC) -lsimavr -lm -lelf $< -o $@

clean:
	rm -f $(PROGNAME).hex $(PROGNAME).bin

