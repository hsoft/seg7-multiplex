import argparse
from functools import partial
from random import randint

from icemu.mcu import ATtiny
from icemu.shiftregisters import SN74HC595
from icemu.seg7 import Segment7, combine_repr
from icemu.pin import Pin
from icemu.sim import Simulation
from icemu.ui import UIScreen

class SerialInput:
    def __init__(self):
        self.ser = Pin('SER', output=True)
        self.clk = Pin('CLK', output=True)

    def begin(self):
        self.clk.setlow()
        self.clk.sethigh()

    def pushserial(self, high):
        self.clk.setlow()
        self.ser.set(bool(high))
        self.clk.sethigh()

    def pushdigit(self, digit, enable_dot):
        assert digit >= 0 and digit < 10
        for i in range(4):
            self.pushserial(digit & (1 << i))
        self.pushserial(enable_dot)


class Circuit(Simulation):
    def __init__(self, max_digits, serial_input, ftdi=False, with_ui=True):
        # it's possible that 8 digits is too much for the simulation to run well at
        # 10x slowdown. You might have to use a 100x slowdown... or use less digits
        super().__init__(usec_value=5)
        self.mcu = self.add_chip(ATtiny())
        self.sr1 = SN74HC595()
        self.sr2 = SN74HC595()
        self.segs = [self.add_chip(Segment7()) for _ in range(max_digits)]
        self.value = randint(1, (10**max_digits)-1)
        if ftdi:
            from icemu.ftdi import FT232H
            self.ftdi = FT232H()

        self.serial_input = serial_input
        self.mcu.pin_B1.wire_to(self.serial_input.ser)
        self.mcu.pin_B2.wire_to(self.serial_input.clk)

        if ftdi:
            self.ftdi.pin_D1.wire_to(self.serial_input.ser)
            self.ftdi.pin_D0.wire_to(self.serial_input.clk)
            # self.ftdi.pin_D2.wire_to(self.mcu.pin_B2)
            # self.ftdi.pin_D3.wire_to(self.mcu.pin_B3)
            # self.ftdi.pin_D4.wire_to(self.mcu.pin_B4)

        self.sr1.pin_SRCLK.wire_to(self.mcu.pin_B0)
        self.sr1.pin_SER.wire_to(self.mcu.pin_B3)
        self.sr1.pin_RCLK.wire_to(self.mcu.pin_B0)

        self.sr2.pin_SRCLK.wire_to(self.mcu.pin_B4)
        self.sr2.pin_SER.wire_to(self.mcu.pin_B3)
        self.sr2.pin_RCLK.wire_to(self.mcu.pin_B0)
        self.sr2.pin_OE.wire_to(self.mcu.pin_B4)

        for seg, sr2pin in zip(self.segs, self.sr2.getpins(self.sr2.OUTPUT_PINS)):
            seg.wirepins(
                self.sr1,
                ['A', 'D', 'E', 'G', 'F', 'DP', 'C', 'B'],
                self.sr1.OUTPUT_PINS,
            )
            seg.pin_VCC.wire_to(sr2pin)


        if with_ui:
            self._setup_ui()
        self.mcu.run_program('seg7multiplex')

        if with_ui:
            self.increase_value(0)

    def _setup_ui(self):
        uiscreen = UIScreen(self)
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
        self.uiscreen = uiscreen

    def _process(self):
        super()._process()
        if hasattr(self, 'uiscreen'):
            self.uiscreen.refresh()

    def stop(self):
        super().stop()
        if hasattr(self, 'uiscreen'):
            self.uiscreen.stop()

    def increase_value(self, amount):
        maxval = (10**len(self.segs)) - 1
        newval = max(0, min(maxval, self.value + amount))
        self.value = newval
        self.serial_input.begin()
        for _ in range(len(self.segs)):
            self.serial_input.pushdigit(newval % 10, False)
            newval //= 10


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--digits', type=int, default=4)
    parser.add_argument('--ftdi', action='store_true')
    args = parser.parse_args()
    circuit = Circuit(max_digits=args.digits, serial_input=SerialInput(), ftdi=args.ftdi)
    circuit.run()

if __name__ == '__main__':
    main()
