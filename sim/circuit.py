from functools import partial
import os
import signal
import time

from icemu.chip import Chip
from icemu.shiftregisters import CD74AC164
from icemu.seg7 import Segment7, combine_repr
from icemu.ui import UIScreen

class ATtiny45(Chip):
    OUTPUT_PINS = ['B0', 'B1', 'B2', 'B3', 'B4']

    def pin_from_int(self, val):
        PinB0 = 0b01000
        PinB1 = 0b01001
        PinB2 = 0b01010
        PinB3 = 0b01011
        PinB4 = 0b01100
        code = {
            PinB0: 'B0',
            PinB1: 'B1',
            PinB2: 'B2',
            PinB3: 'B3',
            PinB4: 'B4',
        }[val]
        return self.getpin(code)


class SerialBuffer:
    def __init__(self, ser, clk):
        self.ser = ser
        self.clk = clk
        self.buffer = []

    def isempty(self):
        return not bool(self.buffer)

    def push(self, value):
        self.buffer.append(bool(value))

    def pull(self):
        if self.isempty():
            return
        val = self.buffer.pop(0)
        self.clk.setlow()
        self.ser.set(bool(val))
        self.clk.sethigh()

class Circuit:
    def __init__(self):
        self.mcu = ATtiny45()
        self.serial_buffer = SerialBuffer(self.mcu.pin_B1, self.mcu.pin_B0)
        self.sr = CD74AC164()
        self.segs = [Segment7() for _ in range(8)]
        self.value = 0

        self.sr.pin_CP.wire_to(self.mcu.pin_B3)
        self.sr.pin_DS1.wire_to(self.mcu.pin_B4)

        self.segs[0].wirepins(
            self.sr,
            ['F', 'G', 'E', 'D', 'C', 'B', 'A', 'DP'],
            ['Q0', 'Q1', 'Q2', 'Q3', 'Q4', 'Q5', 'Q6', 'Q7'],
        )

        self.increase_value(0)

    def pushreset(self):
        for _ in range(5):
            self.serial_buffer.push(True)
        for _ in range(5):
            self.serial_buffer.push(False)

    def pushdigit(self, digit, enable_dot):
        assert digit >= 0 and digit < 10
        for i in range(4):
            self.serial_buffer.push(digit & (1 << i))
        self.serial_buffer.push(enable_dot)

    def delay(self, us):
        begin = time.time()
        end = begin + us / (1000 * 1000)
        while circuit and time.time() < end:
            if uiscreen:
                uiscreen.refresh()

    def increase_value(self, amount):
        self.value += amount
        self.value = max(0, min(9, self.value))
        self.pushreset()
        self.pushdigit(self.value, False)

circuit = None
uiscreen = None

def pinset(pin_number, high):
    pin = circuit.mcu.pin_from_int(pin_number)
    pin.set(high)
    if uiscreen:
        uiscreen.refresh()

def pinishigh(pin_number):
    pin = circuit.mcu.pin_from_int(pin_number)
    return pin.ishigh()

def delay(us):
    circuit.delay(us)

def stop():
    global circuit, uiscreen
    uiscreen.stop()
    circuit = None
    uiscreen = None

def int0_interrupt_check():
    if not circuit.serial_buffer.isempty():
        circuit.serial_buffer.pull()
        return True
    else:
        return False

def main():
    global circuit, uiscreen
    circuit = Circuit()
    uiscreen = UIScreen()
    uiscreen.add_element(
        "LED Matrix output:",
        partial(combine_repr, *circuit.segs[::-1])
    )
    uiscreen.add_element(
        "Raw val:",
        lambda: str(circuit.value)
    )
    uiscreen.add_element(
        "Raw SR:",
        lambda: str(circuit.sr)
    )
    uiscreen.add_action(
        '+', "Increase",
        partial(circuit.increase_value, 1),
    )
    uiscreen.add_action(
        '-', "Decrease",
        partial(circuit.increase_value, -1),
    )
    uiscreen.add_action(
        'q', "Quit",
        partial(os.kill, os.getpid(), signal.SIGINT),
    )
    uiscreen.refresh()

if __name__ == '__main__':
    main()
