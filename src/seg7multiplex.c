#include <stdint.h>
#include <stdio.h>
#ifndef SIMULATION
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#else
#include "../common/sim.h"
#endif

#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/intmath.h"


#define SRCLK PinB2
#define SER PinB3
#define INCLK PinB0
#define SEGCP PinB4
#define RCLK PinB1

// Least significant bit is on Q0
//               BC.FGEDA
#define Seg7_0 0b11010111
#define Seg7_1 0b11000000
#define Seg7_2 0b10001111
#define Seg7_3 0b11001011
#define Seg7_4 0b11011000
#define Seg7_5 0b01011011
#define Seg7_6 0b01011111
#define Seg7_7 0b11000001
#define Seg7_8 0b11011111
#define Seg7_9 0b11011011
#define Seg7_Dot 0b00000100

#define MAX_SER_CYCLES_BEFORE_TIMEOUT 3
#ifndef MAX_DIGITS
#define MAX_DIGITS 8
#endif

static volatile bool refresh_needed;
static volatile uint32_t display_value;
static volatile uint8_t display_dotmask;
static volatile uint8_t digit_count;
static volatile uint8_t ser_input;
static volatile uint8_t ser_input_pos;
static volatile uint8_t ser_timeout;
static uint8_t pin_to_refresh;

static void toggleclk(Pin pin)
{
    pinlow(pin);
    _delay_us(1);
    pinhigh(pin);
}

// LSB goes on q0
static void shiftsend(uint8_t val)
{
    char i;

    for (i=7; i>=0; i--) {
        pinset(SER, val & (1 << i));
        toggleclk(SRCLK);
    }
}

static void senddigits(uint32_t val, uint8_t dotmask)
{
    uint8_t digits[10] = {Seg7_0, Seg7_1, Seg7_2, Seg7_3, Seg7_4, Seg7_5, Seg7_6, Seg7_7, Seg7_8, Seg7_9};
    uint8_t tosend;

    val /= int_pow10(pin_to_refresh);
    tosend = digits[val % 10];
    if (dotmask & (1 << pin_to_refresh)) {
        tosend |= Seg7_Dot;
    }
    pinset(SER, pin_to_refresh == 0);
    toggleclk(SEGCP); // SEGCP is now high, meaning that OE is disabled
    pinlow(RCLK);
    shiftsend(~tosend);
    pinhigh(RCLK);
    pinlow(SEGCP); // OE is now enabled!
    pin_to_refresh++;
    if (pin_to_refresh == MAX_DIGITS) {
        pin_to_refresh = 0;
    }
}

static void push_digit(uint8_t value)
{
    uint32_t amount_to_add;

    if (value & 0b10000) {
        display_dotmask |= (1 << digit_count);
        value &= 0b1111;
    }

    amount_to_add = value * int_pow10(digit_count);
    display_value += amount_to_add;
    digit_count++;
}

static void reset()
{
    display_value = 0;
    display_dotmask = 0;
    digit_count = 0;
    ser_input_pos = 0;
    ser_input = 0;
    ser_timeout = MAX_SER_CYCLES_BEFORE_TIMEOUT;
    pin_to_refresh = 0;
    refresh_needed = false;
}

#ifndef SIMULATION
ISR(INT0_vect)
#else
void seg7multiplex_int0_interrupt()
#endif
{
    if (ser_timeout == 0) {
        reset();
    }

    pininputmode(SER);
    if (pinishigh(SER)) {
        ser_input |= (1 << ser_input_pos);
    }
    pinoutputmode(SER);

    ser_input_pos++;
    if (ser_input_pos == 5) {
        if (ser_input == 0x1f) {
            ser_timeout = 0;
        } else {
            push_digit(ser_input);
            ser_timeout = MAX_SER_CYCLES_BEFORE_TIMEOUT;
        }
        ser_input = 0;
        ser_input_pos = 0;
    }
}

#ifndef SIMULATION
ISR(TIMER0_COMPA_vect)
#else
void seg7multiplex_timer0_interrupt()
#endif
{
    if (ser_timeout == 0) {
        refresh_needed = true;
    } else {
        // We don't refresh while we receive serial signal, but we give ourselves a maximum number
        // of cycle before we say "screw that, you're taking too long.
        ser_timeout--;
    }
}

void seg7multiplex_setup()
{
#ifndef SIMULATION
    sei();
#endif

    pinoutputmode(SER);
    pinoutputmode(SRCLK);
    pinoutputmode(SEGCP);
    pinoutputmode(RCLK);
    pinlow(SEGCP);

    reset();
    refresh_needed = true;

    // Set timer that controls refreshes
    set_timer0_target(1000); // every 1 ms
    set_timer0_mode(TIMER_MODE_INTERRUPT);
}

void seg7multiplex_loop()
{
    if (refresh_needed) {
        refresh_needed = false;
        senddigits(display_value, display_dotmask);
    }
}
