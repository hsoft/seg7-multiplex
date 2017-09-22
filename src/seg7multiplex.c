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


#define SRCLK PinB3
#define SRSER PinB4
#define INCLK PinB0
#define INSER PinB1
#define SEG1 PinB2
#define SEG2 PinB5

// Least significant bit is on Q0
//               XABCDEGF
#define Seg7_0 0b01111101
#define Seg7_1 0b00110000
#define Seg7_2 0b01101110
#define Seg7_3 0b01111010
#define Seg7_4 0b00110011
#define Seg7_5 0b01011011
#define Seg7_6 0b01011111
#define Seg7_7 0b01110000
#define Seg7_8 0b01111111
#define Seg7_9 0b01111011
#define Seg7_Dash 0b00000010
#define Seg7_Dot 0b10000000

#define MAX_SER_CYCLES_BEFORE_TIMEOUT 3

static volatile bool refresh_needed;
static volatile uint16_t display_value;
static volatile uint16_t display_dotmask;
static volatile uint8_t digit_count;
static volatile uint8_t ser_input;
static volatile uint8_t ser_input_pos;
static volatile uint8_t ser_timeout;
static uint8_t pin_to_refresh;

static void toggleclk()
{
    pinlow(SRCLK);
    _delay_us(100);
    pinhigh(SRCLK);
}

// LSB goes on q0
static void shiftsend(uint8_t val)
{
    char i;

    for (i=7; i>=0; i--) {
        pinset(SRSER, val & (1 << i));
        toggleclk();
    }
}

static void senddigits(uint16_t val, uint8_t dotmask)
{
    uint8_t digits[10] = {Seg7_0, Seg7_1, Seg7_2, Seg7_3, Seg7_4, Seg7_5, Seg7_6, Seg7_7, Seg7_8, Seg7_9};
    uint8_t tosend[3];
    uint8_t i;

    if (val > 999) {
        dotmask |= 0b100;
    }
    for (i=0; i<=2; i++) {
        tosend[i] = digits[val % 10];
        if (dotmask & (1 << i)) {
            tosend[i] |= Seg7_Dot;
        }
        val /= 10;
    }
    pinlow(SEG1);
    pinlow(SEG2);
    if (pin_to_refresh == 0) {
        shiftsend(~tosend[0]);
        pinhigh(SEG1);
        if (digit_count > 1) {
            pin_to_refresh = 1;
        }
    } else if (pin_to_refresh == 1) {
        shiftsend(~tosend[1]);
        pinhigh(SEG2);
        pin_to_refresh = 0;
    }
}

static void push_digit(uint8_t value)
{
    uint8_t i;

    if (value & 0b10000) {
        display_dotmask |= (1 << digit_count);
        value &= 0b1111;
    }

    for (i=0; i<digit_count; i++) {
        value *= 10;
    }
    display_value += value;
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

    if (pinishigh(INSER)) {
        ser_input |= (1 << ser_input_pos);
    }
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

    pinoutputmode(SRSER);
    pinoutputmode(SRCLK);
    pinoutputmode(SEG1);
    pinoutputmode(SEG2);

    reset();
    refresh_needed = true;

    // Set timer that controls refreshes
    set_timer0_target(F_CPU / 1000); // every 1 ms
    set_timer0_mode(TIMER_MODE_INTERRUPT);
}

void seg7multiplex_loop()
{
    if (refresh_needed) {
        refresh_needed = false;
        senddigits(display_value, display_dotmask);
    }
}
