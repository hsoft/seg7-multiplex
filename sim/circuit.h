#pragma once
#include "icemu.h"

#ifndef MAX_DIGITS
#define MAX_DIGITS 4
#endif

typedef struct {
    ICeChip mcu;
    ICeChip sr1;
    ICeChip sr2;
    ICeChip segs[MAX_DIGITS];
    ICePin *PB0;
    ICePin *PB1;
    ICePin *PB2;
    ICePin *PB3;
    ICePin *PB4;
} Seg7Multiplex;

void seg7multiplex_circuit_init(Seg7Multiplex *circuit, ICePin *ser, ICePin *clk);
