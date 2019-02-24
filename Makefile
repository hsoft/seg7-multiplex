PROGNAME = seg7multiplex
AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 
TARGETS = $(PROGNAME).S.hex drive.S.hex

# Rules

.PHONY: send senddrive all clean

all: $(TARGETS)
	@echo Done!

send: $(PROGNAME).S.hex
senddrive: drive.S.hex
send senddrive:
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$<

$(PROGNAME).S.hex: $(PROGNAME).S
drive.S.hex: drive.S
$(TARGETS):
	avra -I /usr/include/avr $<

simulation: sim.c $(TARGETS)
	$(CC) -lsimavr -lm -lelf $< -o $@

clean:
	rm -f $(TARGETS) *.S.eep.hex *.S.cof *.S.obj simulation
