from functools import partial
import os
import signal

from icemu.chip import Chip
from icemu.shiftregisters import CD74AC164
from icemu.seg7 import Segment7, combine_repr
from icemu.decoders import SN74HC138
from icemu.gates import SN74HC14
from icemu.ui import UIScreen

class ATtiny45(Chip):
    OUTPUT_PINS = ['B0', 'B1', 'B2', 'B3', 'B4', 'B5']

    def pin_from_int(self, val):
        PinB0 = 0b01000
        PinB1 = 0b01001
        PinB2 = 0b01010
        PinB3 = 0b01011
        PinB4 = 0b01100
        PinB5 = 0b01101
        code = {
            PinB0: 'B0',
            PinB1: 'B1',
            PinB2: 'B2',
            PinB3: 'B3',
            PinB4: 'B4',
            PinB5: 'B5',
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


class Timer:
    def __init__(self):
        self.cnt = 0
        self.target = 0

    def delay(self, us):
        if self.target > 0:
            self.cnt += us

    def check(self):
        if self.target <= 0:
            return False
        if self.cnt >= self.target:
            self.cnt -= self.target
            return True
        else:
            return False

    def set_target(self, target):
        self.target = target
        self.cnt = 0


class Circuit:
    def __init__(self):
        self.mcu = ATtiny45()
        self.serial_buffer = SerialBuffer(self.mcu.pin_B1, self.mcu.pin_B0)
        self.sr = CD74AC164()
        self.segs = [Segment7() for _ in range(3)]
        self.dec = SN74HC138()
        self.inv = SN74HC14()
        self.timer0 = Timer()
        self.value = 123

        self.sr.pin_CP.wire_to(self.mcu.pin_B3)
        self.sr.pin_DS1.wire_to(self.mcu.pin_B4)

        self.dec.pin_A.wire_to(self.mcu.pin_B2)
        self.dec.pin_B.wire_to(self.mcu.pin_B5)

        for decpin, invpin in zip(self.dec.getpins(self.dec.OUTPUT_PINS), self.inv.getpins(self.inv.INPUT_PINS)):
            invpin.wire_to(decpin)

        for seg, invpin in zip(self.segs, self.inv.getpins(self.inv.OUTPUT_PINS)):
            seg.wirepins(
                self.sr,
                ['F', 'G', 'E', 'D', 'C', 'B', 'A', 'DP'],
                ['Q0', 'Q1', 'Q2', 'Q3', 'Q4', 'Q5', 'Q6', 'Q7'],
            )
            seg.vcc.wire_to(invpin)

        self.increase_value(0)

    def pushstop(self):
        for _ in range(5):
            self.serial_buffer.push(True)

    def pushdigit(self, digit, enable_dot):
        assert digit >= 0 and digit < 10
        for i in range(4):
            self.serial_buffer.push(digit & (1 << i))
        self.serial_buffer.push(enable_dot)

    def delay(self, us):
        if uiscreen:
            uiscreen.tick(us)
            uiscreen.refresh()
        self.timer0.delay(us)
        for seg in self.segs:
            seg.tick(us)
        if uiscreen:
            uiscreen.refresh()

    def increase_value(self, amount):
        newval = max(0, min(999, self.value + amount))
        self.value = newval
        self.pushdigit(newval % 10, False)
        if newval > 9:
            self.pushdigit((newval // 10) % 10, False)
        if newval > 99:
            self.pushdigit((newval // 100) % 10, False)
        self.pushstop()

circuit = None
uiscreen = None

def pinset(pin_number, high):
    pin = circuit.mcu.pin_from_int(pin_number)
    pin.set(high)

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

def set_timer0_target(ticks):
    circuit.timer0.set_target(ticks)

def set_timer0_mode(mode):
    pass

def timer0_interrupt_check():
    return circuit.timer0.check()

def main():
    global circuit, uiscreen
    circuit = Circuit()
    uiscreen = UIScreen(refresh_rate_us=(100 * 1000)) # 100ms
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
    uiscreen.add_element(
        "SEG pins:",
        lambda: " ".join(str(seg.vcc) for seg in circuit.segs)
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
