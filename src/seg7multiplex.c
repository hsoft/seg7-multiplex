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


#define SRCLK PinB3
#define SRSER PinB4
#define INCLK PinB0
#define INSER PinB1

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

static uint16_t display_value;
static uint8_t digit_count;
static uint8_t ser_input;
static uint8_t ser_input_pos;
static bool ser_last_input_all_high;

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
    shiftsend(~tosend[0]);
}

static void push_digit(uint8_t value)
{
    uint8_t dotmask = 0;

    if (value & (1 << 5)) {
        dotmask = 1;
        value &= (1 << 5);
    }

    senddigits(value, dotmask);
}

static void reset()
{
    ser_input_pos = 0;
    ser_input = 0;
    ser_last_input_all_high = false;
}

#ifndef SIMULATION
ISR(INT0_vect)
#else
void seg7multiplex_int0_interrupt()
#endif
{
    if (pinishigh(INSER)) {
        ser_input |= (1 << ser_input_pos);
    }
    ser_input_pos++;
    if (ser_input_pos == 5) {
        if (ser_input == 0x1f) {
            ser_last_input_all_high = true;
        } else if ((ser_input == 0) && ser_last_input_all_high) {
            reset();
        } else {
            push_digit(ser_input);
            ser_last_input_all_high = false;
        }
        ser_input = 0;
        ser_input_pos = 0;
    }
}

void seg7multiplex_setup()
{
#ifndef SIMULATION
    sei();
#endif

    pinoutputmode(SRSER);
    pinoutputmode(SRCLK);

    reset();
}

void seg7multiplex_loop()
{
}
