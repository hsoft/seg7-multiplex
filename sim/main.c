#include <stdio.h>
#include <unistd.h>
#include "../src/seg7multiplex.h"
#include "icemu/capi/icemu.h"
#include "icemu/capi/avr/attiny.h"

int main(void)
{
    seg7multiplex_setup();

    while (1) {
        if (icemu_check_interrupt() == ICEMU_INT0) {
            seg7multiplex_int0_interrupt();
            icemu_end_interrupt();
        }
        if (icemu_check_timer(ICEMU_TIMER0)) {
            seg7multiplex_timer0_interrupt();
        }
        seg7multiplex_loop();
        icemu_process_messages();
    }
}
