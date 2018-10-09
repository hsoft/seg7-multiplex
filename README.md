# Multiplexed 7-segment display array

This board is a multiplexed array of 4 7-segment displays that receives its
input through a custom serial protocol. The board uses a USB-A connector to
connect `VCC`, `GND` and its two serial pins. It's multiplexed to minimize the
number of pins and ICs we need to use. We end up only needing an ATtiny45 and
2 ICs to drive pu the 4 displays.

The goal is to be able to offload the "digit display" complexity off little
devices I'm planning to do. Of course, there's tons of ready-to-use displays
out there but I'd like to explore simple and minimal designs.

I'm also hoping to have something that draws minimal current. My first
prototype works on 3V and draws less than 6mA. For reference, the backlight of
a typical 2x8 LCD screen draws something like 20-40mA.

## Serial protocol

It works by serially sending it 5 bits of data using `INSER` and `INCLK`, the
first 4 bits being an encoded digit from `0` to `9` (starting with the least
significant bit) and ending with a "dot" bit that determines if the dot is
shown or not.

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

## Prototypes

### v1

My first prototype was using two shift registers to multiplex my 7-segments
digits. It worked rather well but my craftmanship was lacking. Soldering wasn't
good and the board was too cramped. I also forgot (or didn't know about) to add
decoupling capacitors next to my ICs and I think that this is why that prototype
wasn't working perfectly (it would receive the wrong data through serial and
would even sometimes have a corrupt display (missing/extra segment) about 10% of
the time). Also, to get that figure to 10% from 50%, I had to lower my serial
clock timing to 100us (a minimum of 100us between serial clocks). That seems a
bit high to me. Something is fishy.

As a software developer, I'm used to be on solid grounds. If the code is
logical, generally, things work. Here I have to learn to deal with timing and
power fluctuation issues. These present interesting challenges.

Because my board was too cramped, I needed to make a new prototype altogether
(with decoupling capacitors this time) and I thought that if I was going to do
that, I might as well try another design, that is, my v2.

You can see the code for this v1 in the `v1` branch of this repo. I have no
KiCAD schema though. Never bothered. Will do for v2 though.

### v2

Although I think that my design for v1 was globally sound, I think in
retrospective that the MCU did too many things. The less things the MCU does,
the more logic you offload to ICs, the more robust your design is going to be (I
think). After all, people designing ICs went through much more trouble to make
sure it worked well than I did for my prototype...

So that's why I went with a new multiplex design with 3 ICs:

1. 7-segment decoder hooked to my displays
2. binary counter hooked to the decoder
3. shift register hooked to displays' `VCC` (and `DP`)

With the decoder + counter, sending a particular digit mask to the display is
only a matter of clocking the counter, so it's only two pin changes. That is
much much less than the number of pins we have to change to send a particular
digit mask into a shift register.

I could use a decade counter to only cycle through the first 10 glyphs of my
decoder's 16, but as I looked as extra glyphs, I realized (I really wasn't sure
what they were supposed to be for initially) that they're probably optimized for
multiplexing, that is, that the 5 extra masks (last one is blank) are masks that
"fit" multiple digits. For example, the 11th mask is the lower half of `5` which
also fits 6 and 8.

Therefore, when cycling through masks, you can enable `VCC` not only for exact
digit matches but also partial matches and, in the end, you've refreshed your
multiplexed displays properly. I'm thinking that 1ms per cycle should be good
enough to keep us under the 10ms threshold (at which point the eye starts to see
a flicker) even if we cycle through 16 digit masks.

One problem though is that the 7-segments decoder doesn't handle `DP`, which I'd
like to support. This is why I'm thinking of bringing down (theoretical) digit
support from 8 to 4 and hook 4 pins of my shift register directly to my `DP`
pins. This way, I'll handle `DP` status completely outside of the multiplexing
process.

With the `ATtiny45`, I only have 3 free output pins (2 pins are used for serial
communication). That requires creativity to handle all my ICs with only 3 pins,
but I think I have a good idea:

1. pins 1 and 2 hooked to my shift register
2. pin 3 to my binary counter
3. pin 3 also hooked to my shift register's `OE` pin (output enable).

I need to control `OE` because there's always going to be a slight window of
time during which the decoder cycled and the shift register hasn't updated `VCC`
pin yet. If I let `OE` on, then my displays are going to have "ghost" branches.
The pin clocking the binary counter is going to be in position to enable `OE`
most of the time and right when we will toggle the binary counter clock, `OE`
will become disabled. Then, I'll update the shift register and toggle `OE` back
(which will not trigger a binary counter update since it takes a full on/off
cycle to trigger). I haven't looked yet whether I'm going to be needing an
inverter for this. If yes, I'll probably look for a shift register that allows
me to do that without an inverter (an extra IC just for this seem a bit much).

**Conclusion**: It went well! It was the first time I ordered a PCB from a
KiCAD design. Of course, I made some mistakes in it and a lot of cowboy
soldering was necessary to adjust, but in the end, I had a fully functional
prototype! Unlike v1, it received data properly and displayed it correctly!

However, I'm not done yet, I need a v3 because 3 ICs is too many: I'd like to
get this package in a 55mmx30mmx20mm enclosure. I'll need to go "double side"
and remove an IC.

### v3

With v3, I get rid of the counter. Instead of using 4 bits of the SR for DP,
I connect them to the BCD. Then, I connect SER to DP add a refresh "round"
especially for DP. This means that we generally have to keep SER high to avoid
falsely lighting DPs.

It still offloads most of the complexity to the BCD and gets rid of an IC.
Moreover, althrough I didn't experience it, I felt that the CNT approach was
flaky: the counter **had** to stay in sync with the MCU and the MCU had no way
to reset the counter to enforce syncing (not enough pins).

## Simulation

This project can be simulated on a desktop computer! The simulation uses
[icemu][icemu]. To run it, `cd` into `sim` and run `make`, then
`./seg7multiplex`. You'll get something like this:

[![asciinema](https://asciinema.org/a/RxFAJOHpEg3R0Vu5M7mUI73sD.png)](https://asciinema.org/a/RxFAJOHpEg3R0Vu5M7mUI73sD)

[icemu]: https://github.com/hsoft/icemu
