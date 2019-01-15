#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/intmath.h"

#define DIGITS 4

#define CLK PinB0
#define SER PinB1

static uint16_t val_to_send = 1234;
volatile static bool should_send = false;

void clock(bool high)
{
    pinlow(CLK);
    pinset(SER, high);
    /* After a little trial-and-error, this is the lowest delay I found with
     * which I wouldn't get occasional erroring out on the other end. I find it
     * a bit high (after all, the the same MCU on the other side, so the same
     * speed). This indicates that the processing of input is more costly than
     * I thought. Interesting...
     */
    _delay_us(300);
    pinhigh(CLK);
}

void digit(uint8_t digit)
{
    if (digit >= 10) {
        return;
    }
    clock(digit & 1);
    clock(digit & (1 << 1));
    clock(digit & (1 << 2));
    clock(digit & (1 << 3));
    clock(false); // no dot
}

void send(uint16_t val)
{
    uint8_t digits[DIGITS];

    /* The computation below doesn't look like much, but it's actually intensive
     * for the MCU. If it has to perform it while clocking has begun, we bust
     * timeout, even if, in general, sending is much easier on the MCU than
     * receiving (see _delay_us() comment in clock()). So, pre-compute before
     * clocking.
     */
    for (int i=0; i < DIGITS; i++) {
        digits[i] = (val / int_pow10(i)) % 10;
    }

    // Empty CLK to begin
    clock(false);
    for (int i=0; i < DIGITS; i++) {
        digit(digits[i]);
    }
}

ISR(TIMER0_COMPA_vect)
{
    should_send = true;
}

int main(void)
{
    sei();
    pinoutputmode(CLK);
    pinoutputmode(SER);
    set_timer0_target(1000L * 1000L); // every second
    set_timer0_mode(TIMER_MODE_INTERRUPT);

    while (1) {
        if (should_send) {
            should_send = false;
            send(val_to_send);
            val_to_send++;
        }
    }
}
