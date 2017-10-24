from functools import partial

from icemu.mcu import ATtiny
from icemu.shiftregisters import SN74HC595
from icemu.seg7 import Segment7, combine_repr
from icemu.pin import Pin
from icemu.ui import SimulationWithUI

class Circuit(SimulationWithUI):
    def __init__(self):
        super().__init__(usec_value=10)
        self.mcu = self.add_chip(ATtiny())
        self.sr1 = SN74HC595()
        self.sr2 = SN74HC595()
        self.segs = [Segment7() for _ in range(8)]
        self.value = 87654321

        self.in_ser = Pin(code='INSER', output=True)
        self.mcu.pin_B4.wire_to(self.in_ser)

        self.sr1.pin_SRCLK.wire_to(self.mcu.pin_B2)
        self.sr1.pin_SER.wire_to(self.mcu.pin_B4)
        self.sr1.pin_RCLK.wire_to(self.mcu.pin_B1)

        self.sr2.pin_SRCLK.wire_to(self.mcu.pin_B3)
        self.sr2.pin_SER.wire_to(self.mcu.pin_B4)
        self.sr2.pin_RCLK.wire_to(self.mcu.pin_B1)
        self.sr2.pin_OE.wire_to(self.mcu.pin_B3)

        for seg, sr2pin in zip(self.segs, self.sr2.getpins(self.sr2.OUTPUT_PINS)):
            seg.wirepins(
                self.sr1,
                ['F', 'G', 'E', 'D', 'C', 'B', 'A', 'DP'],
                self.sr1.OUTPUT_PINS,
            )
            seg.vcc.wire_to(sr2pin)


        self._setup_ui()
        self.mcu.run_program('seg7multiplex')

        self.increase_value(0)

    def _setup_ui(self):
        uiscreen = self.uiscreen
        uiscreen.add_element(
            "LED Matrix output:",
            partial(combine_repr, *self.segs[::-1])
        )
        uiscreen.add_element(
            "Raw val:",
            lambda: str(self.value)
        )
        uiscreen.add_element(
            "MCU:",
            self.mcu.asciiart
        )
        uiscreen.add_element(
            "SR1:",
            self.sr1.asciiart
        )
        uiscreen.add_element(
            "SR2:",
            self.sr2.asciiart
        )
        uiscreen.add_action(
            '+', "Increase",
            partial(self.increase_value, 1),
        )
        uiscreen.add_action(
            '-', "Decrease",
            partial(self.increase_value, -1),
        )
        uiscreen.add_action(
            'q', "Quit",
            self.stop,
        )
        uiscreen.refresh()

    def pushserial(self, high):
        self.in_ser.set(bool(high))
        self.mcu.interrupt(0)

    def pushstop(self):
        for _ in range(5):
            self.pushserial(True)

    def pushdigit(self, digit, enable_dot):
        assert digit >= 0 and digit < 10
        for i in range(4):
            self.pushserial(digit & (1 << i))
        self.pushserial(enable_dot)

    def _process(self):
        super()._process()
        for seg in self.segs:
            seg.tick(self.TIME_RESOLUTION)

    def increase_value(self, amount):
        newval = max(0, min(99999999, self.value + amount))
        self.value = newval
        while newval:
            self.pushdigit(newval % 10, False)
            newval //= 10
        self.pushstop()


def main():
    circuit = Circuit()
    circuit.run()

if __name__ == '__main__':
    main()
