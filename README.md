# Multiplexed 7-segment display array

This board is a multiplexed array of up to 8 7-segment displays with a minimal number of input pins.
Those pins are:

* `SER`: Serial input
* `CLK`: Serial input clock

It works by serially sending it 5 bits of data using `SER` and `CLK`, the first 4 bits being an
encoded digit from `0` to `9` (starting with the least significant bit) and ending with a "dot" bit
that determines if the dot is shown or not.

This is done for each digit that needs to be shown (starting with the rightmost digit). Thus, to
show 2 digits, we send 10 bits.

When you're finished sending serial data, send 5 high bits to signify that you're finished so that
we can resume refreshing the display. There's a max serial inactivity delay of 3 refresh cycles (
one cycle is 1ms). After that, we consider that you're finished sending serial data and your next
digit will be the first of the serie.

The board itself takes care of properly refreshing the displays. We refresh one display every 1ms,
cycling over active displays. We only need to send new digits when they change.
