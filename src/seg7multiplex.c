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


#define SRCLK PinB0
#define SER PinB3
#define SEGCP PinB4
#define INCLK PinB2
#define INSER PinB1

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

#define MAX_SER_CYCLES_BEFORE_TIMEOUT 30
#ifndef MAX_DIGITS
#define MAX_DIGITS 4
#endif

/* ABOUT THE COMPLEXITY OF THIS UNIT
 *
 * Making the choice of an ATtiny MCU greatly limits our available pins and forces us to make
 * interesting compromises. This brought us to a design where the SER pin is shared for both
 * SR1, SR2 *and* input data. I first started developing the "multiplexing display" part of the
 * device with a naive algo that woked well both in simulation and real world, all was good.
 *
 * But then, I wanted to make the serial input work, I thought it would be easy, but no! It turns
 * out that toggling a pin from output to input to output again in an interrupt frequently causes
 * clash with our two SRs (who would have thunk?).
 *
 * In the previous naive algo, 8bit data sent to SR1 was done all in one shot, in a for loop. In
 * between CLK low/high, there was a 1us delay. This approach pretty much guaranteed clashes when
 * serial input would come around. I had to use another approach. This one.
 *
 * In a word, I "atomicised" all operations to much smaller chunks of logic at the cost of
 * increased overall complexity.
 *
 * With this approach, we perform SR1 update in 16 atomic steps, each one being performed (if
 * needed) in separate runloop iterations.
 *
 * Higher priority is given to the reading of serial data coming through the interrupt. This queue
 * really has to be emptied as fast as possible because we don't have control over the speed at
 * which data is coming in.
 *
 * Lower priority is given to the screen refreshing because we have ample time here. It takes 10ms
 * without power for a segment to start showing flickering and we're significantly below that with
 * 4 digits. We could easily support 8.
 *
 * An important lesson learned here: keep code in interrupt routines minimal, really barebone.
 */

static volatile bool refresh_needed;
static volatile bool input_mode;

static uint8_t ser_input;
static uint8_t ser_input_pos;
static uint32_t display_value;
static uint8_t display_dotmask;
static uint8_t digit_count;
static uint8_t ser_timeout;
static uint8_t pin_to_refresh;

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

static SRValueSender sr1_sender;

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

static void init_sr1_sender(uint8_t val)
{
    sr1_sender.val = val;
    sr1_sender.index = 0;
    sr1_sender.going_high = false;
}

// Shift registers usually have CLK minimum delays in the order of 100ns. This algo here assumes
// that the overhead of calling sr1_sender_step() in a runloop results at each call is a delay
// that is more than sufficient for this required delay.
static SRValueSenderStatus sr1_sender_step()
{
    SRValueSenderStatus res;

    if (sr1_sender.index < 8) {
        res = SRValueSenderStatus_Middle;
        if (sr1_sender.going_high) {
            if (sr1_sender.index == 7) {
                res = SRValueSenderStatus_Last;
            }
            pinset(SER, sr1_sender.val & (1 << (7 - sr1_sender.index)));
            pinhigh(SRCLK);
            sr1_sender.going_high = false;
            sr1_sender.index++;
        } else {
            if (sr1_sender.index == 0) {
                res = SRValueSenderStatus_Beginning;
            }
            pinlow(SRCLK);
            sr1_sender.going_high = true;
        }
    } else {
        res = SRValueSenderStatus_Finished;
    }
    return res;
}

static void send_next_digit(uint32_t val, uint8_t dotmask)
{
    uint8_t digits[10] = {Seg7_0, Seg7_1, Seg7_2, Seg7_3, Seg7_4, Seg7_5, Seg7_6, Seg7_7, Seg7_8, Seg7_9};
    uint8_t tosend;

    val /= int_pow10(pin_to_refresh);
    tosend = digits[val % 10];
    if (dotmask & (1 << pin_to_refresh)) {
        tosend |= Seg7_Dot;
    }
    init_sr1_sender(~tosend);
}

// Returns whether we had anything to do at all
static bool perform_display_step()
{
    bool res;

    res = true;

    switch (sr1_sender_step()) {
        case SRValueSenderStatus_Beginning:
            // SEGCP was already low.
            pinset(SER, pin_to_refresh == 0);
            pinhigh(SEGCP);
            break;
        case SRValueSenderStatus_Middle:
            break;
        case SRValueSenderStatus_Last:
            // because RCLK is hard-wired to SRCLK on SR1, we need to perform one more
            // SRCLK "push" at the end to push the buffer up to the output pins. Had I known
            // about the difficulties I would have had with the "shared SER" approach I've
            // tried and then abandoned, I would have used a buffer-less SR here to save us
            // this wart, but now that the prototye is all soldered-up, we'll suck it up.
            pinlow(SRCLK);
            _delay_us(1);
            pinhigh(SRCLK);
            pinlow(SEGCP); // OE is now enabled!
            pin_to_refresh++;
            if (pin_to_refresh == MAX_DIGITS) {
                pin_to_refresh = 0;
            }
            break;
        case SRValueSenderStatus_Finished:
            res = false;
            break;
    }

    return res;
}

static void push_digit(uint8_t value)
{
    uint32_t amount_to_add;

    if (value & 0b10000) {
        display_dotmask |= (1 << digit_count);
        value &= 0b1111;
    }
    if (value >= 10) {
        // Something went wrong, but what can we do about it? not much let's just abort
        return;
    }

    if (digit_count == 0) {
        display_value = value;
    } else {
        amount_to_add = value * int_pow10(digit_count);
        display_value += amount_to_add;
    }
    digit_count++;
}

static void begin_input_mode()
{
    input_mode = true;
    ser_timeout = MAX_SER_CYCLES_BEFORE_TIMEOUT;
    digit_count = 0;
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
    sbi(MCUCR, ISC00);
    sbi(MCUCR, ISC01);
    // enable Pin Change Interrupts
    sbi(GIMSK, INT0);
    sei();
#endif

    pinoutputmode(SER);
    pinoutputmode(SRCLK);
    pinoutputmode(SEGCP);
    pinlow(SEGCP);

    input_mode = false;
    serial_queue_init();
    sr1_sender.index = 8; // begin in "finished" mode;
    display_value = 0;
    display_dotmask = 0;
    ser_timeout = 0;
    pin_to_refresh = 0;
    refresh_needed = true;

    // Set timer that controls refreshes
    set_timer0_target(1000); // every 1 ms
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
                if (digit_count == MAX_DIGITS) {
                    // We're done here
                    end_input_mode();
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
            }
        }
    } else {
        if (!perform_display_step()) {
            // Alright, we're not in the middle of something. Let's see if we have a digit to
            // refresh...
            if (refresh_needed) {
                refresh_needed = false;
                send_next_digit(display_value, display_dotmask);
            }
        }
    }
}
