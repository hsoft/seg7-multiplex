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

To send a new set of digits, the reset code has to be send. This reset code is 5 high bits followed
by 5 low bits. After these 10 bits have been sent in this order, the serial data being received
start from the beginning again.

We send two sets of 5 bits instead of one so that when we send this code, we're sure that our
5bit clocking is in sync.

The board itself takes care of properly refreshing the displays. We only need to send new digits
when they change.
