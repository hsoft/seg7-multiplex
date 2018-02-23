# Communication protocol bench test

As I develop my prototype, I observe that my homemade super simple communication
protocol is a bit flaky. It works a good half of the time, but doesn't receive
any (or the wrong) number the other half.

Before I discover by myself that I should use something that has already been
designed, I'd like to benchtest this thing.

The idea is to load a ATtiny45 with a software that repeatedly expects a
specific number. Whenever it successfully receives that number, it toggles a
pin. Whenever communication fails (a CLK togle was made but ended in receiving
the wrong number or an unterminated communication), another pin is toggled.

I can then wire those pins to binary counter and a couple of LED to have an idea
of the progress.
