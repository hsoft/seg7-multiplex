#include "circuit.h"

void seg7multiplex_loop();

void seg7multiplex_circuit_init(Seg7Multiplex *circuit, ICePin *ser, ICePin *clk)
{
    int i;
    ShiftRegister *sr_lu;

    icemu_ATtiny_init(&circuit->mcu);
    icemu_mcu_set_runloop(&circuit->mcu, seg7multiplex_loop, 20);
    circuit->PB0 = circuit->mcu.pins.pins[0];
    circuit->PB1 = circuit->mcu.pins.pins[1];
    circuit->PB2 = circuit->mcu.pins.pins[2];
    circuit->PB3 = circuit->mcu.pins.pins[3];
    circuit->PB4 = circuit->mcu.pins.pins[4];
    icemu_SN74HC595_init(&circuit->sr);
    icemu_SN7447A_init(&circuit->dec);
    sr_lu = (ShiftRegister *)circuit->sr.logical_unit;
    for (i = 0; i < DIGITS; i++) {
        icemu_seg7_init(&circuit->segs[i]);
    }
    icemu_pin_wireto(circuit->PB1, ser);
    icemu_pin_wireto(circuit->PB2, clk);

    icemu_pin_wireto(circuit->PB3, icemu_chip_getpin(&circuit->sr, "SRCLK"));
    icemu_pin_wireto(circuit->PB4, icemu_chip_getpin(&circuit->sr, "SER"));
    icemu_pin_wireto(circuit->PB0, icemu_chip_getpin(&circuit->sr, "RCLK"));
    icemu_pin_wireto(circuit->PB0, icemu_chip_getpin(&circuit->sr, "OE"));

    icemu_pin_wireto(icemu_chip_getpin(&circuit->sr, "QE"), icemu_chip_getpin(&circuit->dec, "A"));
    icemu_pin_wireto(icemu_chip_getpin(&circuit->sr, "QF"), icemu_chip_getpin(&circuit->dec, "B"));
    icemu_pin_wireto(icemu_chip_getpin(&circuit->sr, "QG"), icemu_chip_getpin(&circuit->dec, "C"));
    icemu_pin_wireto(icemu_chip_getpin(&circuit->sr, "QH"), icemu_chip_getpin(&circuit->dec, "D"));

    for (i = 0; i < DIGITS; i++) {
        icemu_displaydecoder_wireto_seg7(&circuit->dec, &circuit->segs[i]);
        icemu_pin_wireto(icemu_ledmatrix_common_pin(
            &circuit->segs[i]), sr_lu->outputs.pins[i]);
        icemu_pin_wireto(icemu_chip_getpin(&circuit->segs[i], "DP"), circuit->PB4);
    }
}
