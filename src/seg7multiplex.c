#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#ifndef SIMULATION
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#endif

#include "../common/util.h"
#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/intmath.h"


#define SRCLK PinB3
#define SER PinB4
#define RCLK PinB0
#define INCLK PinB2
#define INSER PinB1

#define MAX_SER_CYCLES_BEFORE_TIMEOUT 3
#ifndef DIGITS
#define DIGITS 4
#endif

/* 7-segments multiplexer
 *
 * This code uses an ATtiny to display numbers from 0000 to 9999 on 4
 * 7-segments displays. It does it in a multiplexing way, that is, by quickly
 * and repeatedly refresh displays and take advantage of the fact that the
 * display can be off for up to 10ms before the eye starts to see a flicker.
 *
 * It works by cycling through the 15 glyphs built in the SN74LS47 (we skip the
 * blank glyph). For each glyph, we determine which of the 4 displays should be
 * enabled and set the SN74HC595 to send power the the enabled displays.
 *
 * The number to display is sent serially through INSER and INCLK.
 *
 * Making the choice of an ATtiny MCU greatly limits our available pins and
 * forces us to make interesting compromises. To maximize the responsiveness of
 * serial input and ensure that we don't miss a bit, we have to make sure that
 * the MCU can't be kept busy for too long: there can't be two bits of data set
 * during a single loop() call.
 *
 * All operations are agressively "atomicised" to smal chunks of logic at the
 * cost of increased overall complexity.
 *
 * With this approach, we perform shift register update in 16 atomic steps,
 * each one being performed (if needed) in separate runloop iterations.
 *
 * Higher priority is given to the reading of serial data coming through the
 * interrupt. This queue really has to be emptied as fast as possible because
 * we don't have control over the speed at which data is coming in.
 *
 * Lower priority is given to the screen refreshing because we have ample time
 * here. It takes 10ms without power for a segment to start showing flickering
 * and we're significantly below that with 4 digits. We could easily support 8.
 *
 * LESSON LEARNED 2018-10-07: don't overestimate MCU's capabilities
 *
 * We're so used to powerful computers that we take their power for granted.
 * A couple of additional function calls and ifs can go a long way to make your
 * stuff too slow. In my second prototype, I had a flickering problem. Whatever
 * I would do (make refreshes faster, slower, whatever), my display would
 * flicker badly. I did a little optimization here and there, to no avail. I
 * was about to give up and go back to my first prototype (which at least
 * didn't flicker!) when my last ditch effort paid off: store and compare the
 * display value as an array of digits and give up the idea of matching glyphs
 * 10 to 14. It would avoid a few arithmetic operations, one function jump and
 * a couple of ifs and ... no flicker! This means that the MCU just wasn't fast
 * enough to keep up with the refresh rate.
 * were off was time MCU was doing these
 */

static volatile bool refresh_needed;
static volatile bool input_mode;

static uint8_t ser_input;
static uint8_t ser_input_pos;
// First element of array is rightmost digit
static uint8_t display_digits[DIGITS] = {1, 8, 2, 1};
static uint8_t display_dotmask;
static uint8_t digit_count;
static uint8_t ser_timeout;
static uint8_t current_glyph;

// Here, it is assumed that 16 data element is enough to stay clear of "roundtrips", that is, data
// writing 16 times before we have the change to read anything. The algo using this really must
// properly prioritize the reading of this queue.
typedef struct {
    uint16_t data;
    uint8_t write_index;
    uint8_t read_index;
} SerialQueue;

static volatile SerialQueue serial_queue;

// Status of an operation sending an 8-bit value to a shift register, step by step.
// there are 16 steps, two (clk low, ser+clk high) for each bit.
typedef struct {
    uint8_t val;
    uint8_t index;
    bool going_high;
} SRValueSender;

typedef enum {
    SRValueSenderStatus_Beginning, // We've just started and our CLK pin is low.
    SRValueSenderStatus_Middle, // We're riding.
    SRValueSenderStatus_Last, // We've just performed our last step. our CLK pin is high.
    SRValueSenderStatus_Finished, // We don't have anything to send anymore.
} SRValueSenderStatus;

static SRValueSender sr_sender;

static void serial_queue_init()
{
    serial_queue.data = 0;
    serial_queue.read_index = 0;
    serial_queue.write_index = 0;
}

static void serial_queue_write(bool data)
{
    if (data) {
        serial_queue.data |= 1 << serial_queue.write_index;
    } else {
        serial_queue.data &= ~(1 << serial_queue.write_index);
    }
    serial_queue.write_index++;
    if (serial_queue.write_index == 16) {
        serial_queue.write_index = 0;
    }
}

static bool serial_queue_read(bool *data)
{
    if (serial_queue.read_index == serial_queue.write_index) {
        return false;
    }
    *data = (serial_queue.data & (1 << serial_queue.read_index)) > 0;
    serial_queue.read_index++;
    if (serial_queue.read_index == 16) {
        serial_queue.read_index = 0;
    }
    return true;
}

static void init_sr_sender(uint8_t val)
{
    sr_sender.val = val;
    sr_sender.index = 0;
    sr_sender.going_high = false;
}

// Shift registers usually have CLK minimum delays in the order of 100ns. This algo here assumes
// that the overhead of calling sr_sender_step() in a runloop results at each call is a delay
// that is more than sufficient for this required delay.
static SRValueSenderStatus sr_sender_step()
{
    SRValueSenderStatus res;

    if (sr_sender.index < 8) {
        res = SRValueSenderStatus_Middle;
        if (sr_sender.going_high) {
            if (sr_sender.index == 7) {
                res = SRValueSenderStatus_Last;
            }
            pinset(SER, sr_sender.val & (1 << (7 - sr_sender.index)));
            pinhigh(SRCLK);
            sr_sender.going_high = false;
            sr_sender.index++;
        } else {
            if (sr_sender.index == 0) {
                res = SRValueSenderStatus_Beginning;
            }
            pinlow(SRCLK);
            sr_sender.going_high = true;
        }
    } else {
        res = SRValueSenderStatus_Finished;
    }
    return res;
}

static uint8_t glyph_match_mask(uint8_t glyph)
{
    uint8_t res = 0;
    uint8_t i;

    if (glyph >= 10) {
        // Never matches
        return 0;
    }
    for (i=0; i<DIGITS; i++) {
        if (display_digits[i] == glyph) {
            res |= (1 << i);
        }
    }

    return res;
}

/* Except for skipping a glyph, we don't control RCLK's status here It is
 * exclusively controlled by SR's update routine to maximize the time OE is
 * enabled (low) so that we send a maximum of light to our displays to avoid
 * flickers.
 */
static bool select_next_glyph()
{
    uint8_t tosend;

    current_glyph++;
    if (current_glyph == 0x10) {
        current_glyph = 0;
    }
    tosend = glyph_match_mask(current_glyph);
    tosend |= (~display_dotmask << 4);
    if (tosend != sr_sender.val) {
        init_sr_sender(tosend);
        return true;
    } else {
        // Same digit mask! We don't bother sending the same value to the SR,
        // but we still need to make the counter go forward.
        pinhigh(RCLK);
        _delay_us(1);
        pinlow(RCLK);
        return false;
    }
}

// Returns whether we had anything to do at all
static bool perform_display_step()
{
    bool res;

    res = true;

    switch (sr_sender_step()) {
        case SRValueSenderStatus_Beginning:
            break;
        case SRValueSenderStatus_Middle:
            break;
        case SRValueSenderStatus_Last:
            // Flush out the buffer with RCLK
            pinhigh(RCLK); // Also triggers the counter!
            _delay_us(1);
            pinlow(RCLK); // return to low for OE to be enabled
            break;
        case SRValueSenderStatus_Finished:
            res = false;
            break;
    }

    return res;
}

static void push_digit(uint8_t value)
{
    if (value & 0b10000) {
        display_dotmask |= (1 << digit_count);
        value &= 0b1111;
    }
    if (value >= 10) {
        // Something went wrong, but what can we do about it? not much let's just abort
        return;
    }

    if (digit_count < DIGITS) {
        display_digits[digit_count] = value;
    }
    digit_count++;
}

static void begin_input_mode()
{
    input_mode = true;
    ser_timeout = MAX_SER_CYCLES_BEFORE_TIMEOUT;
    digit_count = 0;
    display_dotmask = 0;
    ser_input_pos = 0;
    ser_input = 0;
}

static void end_input_mode()
{
    input_mode = false;
    ser_timeout = 0;
    serial_queue_init();
}

#ifndef SIMULATION
ISR(INT0_vect)
#else
void seg7multiplex_int0_interrupt()
#endif
{
    if (!input_mode) {
        // first clocking is only to announce data. No actual data is recorded.
        input_mode = true;
    } else {
        serial_queue_write(pinishigh(INSER));
    }
}

#ifndef SIMULATION
ISR(TIMER0_COMPA_vect)
#else
void seg7multiplex_timer0_interrupt()
#endif
{
    refresh_needed = true;
}

void seg7multiplex_setup()
{
#ifndef SIMULATION
    // generate interrupt on rising edge of INT0
    sbi(MCUCR, ISC00);
    sbi(MCUCR, ISC01);
    // enable Pin Change Interrupts
    sbi(GIMSK, INT0);
    sei();
#endif

    pinoutputmode(SER);
    pinoutputmode(SRCLK);
    pinoutputmode(RCLK);

    input_mode = false;
    serial_queue_init();
    sr_sender.index = 8; // begin in "finished" mode;
    display_dotmask = 0;
    ser_timeout = 0;
    current_glyph = 0;
    refresh_needed = true;

    // Set timer that controls refreshes
    set_timer0_target(600); // every 600 us
    set_timer0_mode(TIMER_MODE_INTERRUPT);
}

void seg7multiplex_loop()
{
    bool flag;

    if (input_mode) {
        if (ser_timeout == 0) {
            // We've just started our input mode set it up
            begin_input_mode();
        }
        while (serial_queue_read(&flag)) {
            if (flag) {
                ser_input |= (1 << ser_input_pos);
            }
            ser_input_pos++;
            // We've received data, re-init ser_timer countdown
            ser_timeout = MAX_SER_CYCLES_BEFORE_TIMEOUT;
            if (ser_input_pos == 5) {
                push_digit(ser_input);
                ser_input = 0;
                ser_input_pos = 0;
                if (digit_count == DIGITS) {
                    // We're done here
                    end_input_mode();
                    // Return now so we don't execute the ser_timeout code
                    // below. Doing so after end_input_mode() makes
                    // ser_timeout underflow to 0xff.
                    return;
                }
            }
        }
        // We don't refresh while we receive serial signal, but we give ourselves a maximum number
        // of cycle before we say "screw that, you're taking too long".
        if (refresh_needed) {
            refresh_needed = false;
            ser_timeout--;
            if (ser_timeout == 0) {
                end_input_mode();
                // highlight the leftmost dot to indicate error in the previous
                // reception.
                display_dotmask = 0x1;
            }
        }
    } else {
        while (perform_display_step()) { if (input_mode) return; }
        if (refresh_needed) {
            while (!select_next_glyph()){ if (input_mode) return; }
            refresh_needed = false;
        }
    }
}
