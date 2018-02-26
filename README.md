# Multiplexed 7-segment display array

This board is a multiplexed array of up to 8 7-segment displays with a minimal
number of input pins. It's multiplexed to minimize the number of pins and ICs
we need to use. We end up only needing an ATtiny45 and two shift registers to
drive up to 8 digits.

The goal is to be able to offload the "digit display" complexity off little
devices I'm planning to do. Of course, there's tons of ready-to-use displays
out there but I'd like to explore simple and minimal designs.

I'm also hoping to have something that draws minimal current. My first
prototype works on 3V and draws less than 6mA. For reference, the backlight of
a typical 2x8 LCD screen draws something like 20-40mA.

The idea is to use the 4 wires of USB connectors to power the display and send
serial data using a homemade protocol that tells it what number to show.

## Serial protocol

It works by serially sending it 5 bits of data using `SER` and `CLK`, the first
4 bits being an encoded digit from `0` to `9` (starting with the least
significant bit) and ending with a "dot" bit that determines if the dot is shown
or not.

This is done for each digit that needs to be shown (starting with the rightmost
digit). Thus, to show 2 digits, we send 10 bits.

The number of digits being used by the multiplexer has to be known by the sender
because the multiplexer expects exactly the number of bits it needs to fill
all its digits.

I could have gone for sending straight binary values instead of digits but I
thought it would be nice to eventually add support for special values (dash,
empty, etc)

The board itself takes care of properly refreshing the displays. We refresh one
display every 1ms, cycling over active displays. We only need to send new digits
when they change.

## Prototype

I have a prototype that works rather well. There's a slight flicker when the
value changes but when it doesn't the display is much less flickery.

The MCU is an `ATtiny45` and runs at `1MHZ`. It's plenty fast, to do its
refreshing job while receving serial data (but the code had to be carefully
crafted to minimize the maximum possible time of a runloop and make sure that
display refreshes didn't interfer with serial input triggers), but that serial
data clocking speed has to be slow enough. My thinkering made me place that
speed at `100us` between `CLK` pin changes.

`100us` seems quite high to me though, but as I lower that delay, my rate of
serial errors goes up. I'll have to investigate.

When running under `5V` it draws between `5.5ma` and `5.8ma` and its consumption
doesn't seem to be linked to whether it receives serial data. The LED seem to be
using the most of the current because when I measured the `ATtiny45` chip alone
I got `0.6mA`. When running on 2 AA batteries (`2.4V`), current drops to `2.8mA`
(but the LED are significantly dimmer).

## Simulation

This project can be simulated on a desktop computer! The simulation uses
[icemu][icemu]. To run it, `cd` into `sim` and run `make`, then
`./seg7multiplex`. You'll get something like this:

[![asciinema](https://asciinema.org/a/WsYhXc1VcgfmkKZ8SAT18xYjv.png)](https://asciinema.org/a/WsYhXc1VcgfmkKZ8SAT18xYjv)
[icemu]: https://github.com/hsoft/icemu
