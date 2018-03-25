#pragma once
#include "icemu.h"

#define DIGITS 4

typedef struct {
    ICeChip mcu;
    ICeChip sr;
    ICeChip cnt;
    ICeChip dec;
    ICeChip segs[DIGITS];
    ICePin *PB0;
    ICePin *PB1;
    ICePin *PB2;
    ICePin *PB3;
    ICePin *PB4;
} Seg7Multiplex;

void seg7multiplex_circuit_init(Seg7Multiplex *circuit, ICePin *ser, ICePin *clk);
