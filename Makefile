PROGNAME = seg7multiplex
AVRDUDEMCU ?= t45
AVRDUDEARGS ?= -c usbtiny -P usb 
TARGET = $(PROGNAME).S.hex

# Rules

.PHONY: send all clean

all: $(TARGET)
	@echo Done!

send: $(TARGET)
	avrdude $(AVRDUDEARGS) -p $(AVRDUDEMCU) -U flash:w:$(TARGET)

$(TARGET): $(PROGNAME).S
	avra -I /usr/include/avr $<

simulation: sim.c $(TARGET)
	$(CC) -lsimavr -lm -lelf $< -o $@

clean:
	rm -f $(TARGET) $(PROGNAME).S.eep.hex $(PROGNAME).S.cof $(PROGNAME).S.obj simulation
