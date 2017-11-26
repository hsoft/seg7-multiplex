#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "../common/pin.h"
#include "../common/timer.h"
#include "../common/intmath.h"
#include "icemu.h"

void seg7multiplex_int0_interrupt();
void seg7multiplex_timer0_interrupt();
void seg7multiplex_setup();
void seg7multiplex_loop();

Chip mcu;
Chip sr1;
Chip sr2;
Chip segs[MAX_DIGITS];
Pin ser;
Pin clk;
unsigned int display_val = 1234;

/* Utils */
static Pin* getpin(PinID pinid)
{
    switch (pinid) {
        case PinB0: return mcu.pins.pins[0];
        case PinB1: return mcu.pins.pins[1];
        case PinB2: return mcu.pins.pins[2];
        case PinB3: return mcu.pins.pins[3];
        case PinB4: return mcu.pins.pins[4];
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
    seg7multiplex_loop(); // let the runloop breathe a little bit.
}

static void push_number(uint32_t val)
{
    int i;

    // we start with an empty CLK to begin;
    push_serial(false);

    for (i = 0; i < MAX_DIGITS; i++) {
        push_digit((val / int_pow10(MAX_DIGITS - i - 1)) % 10, false);
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
    icemu_mcu_add_timer(&mcu, usecs, seg7multiplex_timer0_interrupt);
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
    int i, j;
    char * segorder[] = {"A", "D", "E", "G", "F", "DP", "C", "B"};
    ShiftRegister *sr1_lu;
    ShiftRegister *sr2_lu;

    icemu_ATtiny_init(&mcu);
    icemu_SN74HC595_init(&sr1);
    sr1_lu = (ShiftRegister *)sr1.logical_unit;
    icemu_SN74HC595_init(&sr2);
    sr2_lu = (ShiftRegister *)sr2.logical_unit;
    for (i = 0; i < MAX_DIGITS; i++) {
        icemu_seg7_init(&segs[i]);
    }
    icemu_pin_init(&ser, NULL, "SER", true);
    icemu_pin_init(&clk, NULL, "CLK", true);

    icemu_pin_wireto(getpin(PinB1), &ser);
    icemu_pin_wireto(getpin(PinB2), &clk);

    icemu_pin_wireto(getpin(PinB0), icemu_chip_getpin(&sr1, "SRCLK"));
    icemu_pin_wireto(getpin(PinB3), icemu_chip_getpin(&sr1, "SER"));
    icemu_pin_wireto(getpin(PinB0), icemu_chip_getpin(&sr1, "RCLK"));

    icemu_pin_wireto(getpin(PinB4), icemu_chip_getpin(&sr2, "SRCLK"));
    icemu_pin_wireto(getpin(PinB3), icemu_chip_getpin(&sr2, "SER"));
    icemu_pin_wireto(getpin(PinB0), icemu_chip_getpin(&sr2, "RCLK"));
    icemu_pin_wireto(getpin(PinB4), icemu_chip_getpin(&sr2, "OE"));

    for (i = 0; i < MAX_DIGITS; i++) {
        for (j = 0; j < 8; j++) {
            icemu_pin_wireto(sr1_lu->outputs.pins[j], icemu_chip_getpin(&segs[i], segorder[j]));
        }
        icemu_pin_wireto(icemu_ledmatrix_vcc(&segs[i]), sr2_lu->outputs.pins[i]);
    }

    seg7multiplex_setup();
    icemu_mcu_add_interrupt(
        &mcu, getpin(PinB2), INTERRUPT_ON_RISING, seg7multiplex_int0_interrupt);
    icemu_sim_init(50, seg7multiplex_loop);
    icemu_sim_add_action('+', "(+) Increase Value", increase_value);
    icemu_sim_add_action('-', "(-) Decrease Value", decrease_value);
    icemu_sim_add_chip(&mcu);
    icemu_ui_add_element("MCU", &mcu);
    icemu_ui_add_element("SR1", &sr1);
    icemu_ui_add_element("SR2", &sr2);
    for (i = 0; i < MAX_DIGITS; i++) {
        icemu_sim_add_chip(&segs[i]);
        icemu_ui_add_element("", &segs[i]);
    }
    push_number(display_val);
    icemu_sim_run();
    return 0;
}
