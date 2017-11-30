#include "circuit.h"

void seg7multiplex_loop();

void seg7multiplex_circuit_init(Seg7Multiplex *circuit, ICePin *ser, ICePin *clk)
{
    int i, j;
    char * segorder[] = {"A", "D", "E", "G", "F", "DP", "C", "B"};
    ShiftRegister *sr1_lu;
    ShiftRegister *sr2_lu;

    icemu_ATtiny_init(&circuit->mcu);
    icemu_mcu_set_runloop(&circuit->mcu, seg7multiplex_loop, 50);
    circuit->PB0 = circuit->mcu.pins.pins[0];
    circuit->PB1 = circuit->mcu.pins.pins[1];
    circuit->PB2 = circuit->mcu.pins.pins[2];
    circuit->PB3 = circuit->mcu.pins.pins[3];
    circuit->PB4 = circuit->mcu.pins.pins[4];
    icemu_SN74HC595_init(&circuit->sr1);
    sr1_lu = (ShiftRegister *)circuit->sr1.logical_unit;
    icemu_SN74HC595_init(&circuit->sr2);
    sr2_lu = (ShiftRegister *)circuit->sr2.logical_unit;
    for (i = 0; i < MAX_DIGITS; i++) {
        icemu_seg7_init(&circuit->segs[i]);
    }
    icemu_pin_wireto(circuit->PB1, ser);
    icemu_pin_wireto(circuit->PB2, clk);

    icemu_pin_wireto(circuit->PB0, icemu_chip_getpin(&circuit->sr1, "SRCLK"));
    icemu_pin_wireto(circuit->PB3, icemu_chip_getpin(&circuit->sr1, "SER"));
    icemu_pin_wireto(circuit->PB0, icemu_chip_getpin(&circuit->sr1, "RCLK"));

    icemu_pin_wireto(circuit->PB4, icemu_chip_getpin(&circuit->sr2, "SRCLK"));
    icemu_pin_wireto(circuit->PB3, icemu_chip_getpin(&circuit->sr2, "SER"));
    icemu_pin_wireto(circuit->PB0, icemu_chip_getpin(&circuit->sr2, "RCLK"));
    icemu_pin_wireto(circuit->PB4, icemu_chip_getpin(&circuit->sr2, "OE"));

    for (i = 0; i < MAX_DIGITS; i++) {
        for (j = 0; j < 8; j++) {
            icemu_pin_wireto(
                sr1_lu->outputs.pins[j],
                icemu_chip_getpin(&circuit->segs[i], segorder[j]));
        }
        icemu_pin_wireto(icemu_ledmatrix_vcc(&circuit->segs[i]), sr2_lu->outputs.pins[i]);
    }
}
