PROGNAME = ftdiclient
OBJS = ftdiclient.o ../common/intmath.o

TO_CLEAN = $(OBJS) $(PROGNAME)

CFLAGS = -I. -Wall `pkg-config --cflags libftdi1` -c
LDFLAGS = `pkg-config --libs --static ncurses libftdi1`

# Rules
%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

all: $(PROGNAME)

$(PROGNAME): $(OBJS)
	$(CC) $+ -o $@ $(LDFLAGS)

clean:
	rm -f $(TO_CLEAN)

