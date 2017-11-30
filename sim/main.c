#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/intmath.h"
#include "icemu.h"
#include "circuit.h"

void seg7multiplex_int0_interrupt();
void seg7multiplex_timer0_interrupt();
void seg7multiplex_setup();
void seg7multiplex_loop();

static Seg7Multiplex circuit;
static ICePin ser;
static ICePin clk;
unsigned int display_val = 1234;

/* Utils */
static ICePin* getpin(PinID pinid)
{
    switch (pinid) {
        case PinB0: return circuit.PB0;
        case PinB1: return circuit.PB1;
        case PinB2: return circuit.PB2;
        case PinB3: return circuit.PB3;
        case PinB4: return circuit.PB4;
        default: return NULL;
    }
}

static void push_serial(bool high)
{
    icemu_pin_set(&clk, false);
    icemu_pin_set(&ser, high);
    icemu_pin_set(&clk, true);
}

static void push_digit(uint8_t digit, bool enable_dot)
{
    assert(digit < 10);

    push_serial(digit & 1);
    push_serial(digit & (1 << 1));
    push_serial(digit & (1 << 2));
    push_serial(digit & (1 << 3));
    push_serial(enable_dot);
}

static void push_number(uint32_t val)
{
    int i;

    // we start with an empty CLK to begin;
    push_serial(false);

    for (i = 0; i < MAX_DIGITS; i++) {
        push_digit((val / int_pow10(MAX_DIGITS - i - 1)) % 10, false);
        // let the runloop breathe a little
        seg7multiplex_loop();
    }
}

/* Layer impl */
void pinset(PinID pinid, bool high)
{
    icemu_pin_set(getpin(pinid), high);
}

void pinlow(PinID pinid)
{
    pinset(pinid, false);
}

void pinhigh(PinID pinid)
{
    pinset(pinid, true);
}

bool pinishigh(PinID pinid)
{
    return getpin(pinid)->high;
}

void pinoutputmode(PinID pinid)
{
    getpin(pinid)->output = true;
}

bool set_timer0_target(unsigned long usecs)
{
    icemu_mcu_add_timer(&circuit.mcu, usecs, seg7multiplex_timer0_interrupt);
    return true;
}

void set_timer0_mode(TIMER_MODE mode)
{
}

/* Main */
void increase_value()
{
    display_val++;
    push_number(display_val);
}

void decrease_value()
{
    display_val--;
    push_number(display_val);
}

int main()
{
    int i;

    icemu_pin_init(&ser, NULL, "SER", true);
    icemu_pin_init(&clk, NULL, "CLK", true);

    seg7multiplex_circuit_init(&circuit, &ser, &clk);

    seg7multiplex_setup();
    icemu_mcu_add_interrupt(
        &circuit.mcu, getpin(PinB2), ICE_INTERRUPT_ON_RISING, seg7multiplex_int0_interrupt);
    icemu_sim_init();
    icemu_sim_add_action('+', "(+) Increase Value", increase_value);
    icemu_sim_add_action('-', "(-) Decrease Value", decrease_value);
    icemu_ui_add_element("MCU", &circuit.mcu);
    icemu_ui_add_element("SR1", &circuit.sr1);
    icemu_ui_add_element("SR2", &circuit.sr2);
    for (i = 0; i < MAX_DIGITS; i++) {
        icemu_ui_add_element("", &circuit.segs[i]);
    }
    push_number(display_val);
    icemu_sim_run();
    return 0;
}
