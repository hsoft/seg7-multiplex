PROGNAME = seg7multiplex
AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 
TARGETS = $(PROGNAME).hex drive.hex

# Rules

.PHONY: send senddrive all clean

all: $(TARGETS)
	@echo Done!

send: $(PROGNAME).hex
senddrive: drive.hex
send senddrive:
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$<

$(PROGNAME).hex: $(PROGNAME).S
drive.hex: drive.S
$(TARGETS):
	avra -o $@ $<

simulation: sim.c $(TARGETS)
	$(CC) -lsimavr -lm -lelf $< -o $@

clean:
	rm -f $(TARGETS) *.S.eep.hex *.S.cof *.S.obj simulation
