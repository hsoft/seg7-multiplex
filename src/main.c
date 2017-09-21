#include "seg7multiplex.h"

int main(void)
{
    seg7multiplex_setup();

    while (1) {
        seg7multiplex_loop();
    }
}
