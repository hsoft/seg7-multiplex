#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/seg7serial.h"

#define CLK PinB0
#define SER PinB1

static uint16_t val_to_send = 2345;
volatile static bool should_send = false;

ISR(TIMER0_COMPA_vect)
{
    should_send = true;
}

int main(void)
{
    sei();
    seg7serial_setpins(CLK, SER);
    set_timer0_target(1000L * 1000L); // every second
    set_timer0_mode(TIMER_MODE_INTERRUPT);

    while (1) {
        if (should_send) {
            should_send = false;
            seg7serial_send(val_to_send);
            val_to_send++;
        }
    }
}
