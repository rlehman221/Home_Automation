 # MCP23008.py
 # Date: June-08-2018
 # Created by: Ryan Lehman

from Adafruit_I2C import Adafruit_I2C
import smbus
import time



MCP23008_IODIRA = 0x00 # Port A direction register. Write a 0 to make a pin an output, a 1 to make it an input
MCP23008_GPIOA  = 0x09 # Register Address of Port A - read data from or write output data to this port
MCP23008_GPPUA  = 0x06 # Register to enable the internal pull up resistors on Port A. 1 = pull up enabled
MCP23008_OLATA  = 0x0A # Output Latch Register - modifies the output latches 

class MCP23008(object):
    OUTPUT = 0
    INPUT = 1

    def __init__(self, address, num_gpios, busnum=-1):
        assert num_gpios >= 0 and num_gpios <= 8, "Number of GPIOs must be between 0 and 16"
        self.i2c = Adafruit_I2C(address=address, busnum=busnum)
        self.address = address
        self.num_gpios = num_gpios

        # set default configuration
        self.i2c.write8(MCP23008_IODIRA, 0xFF)  # all inputs on port A
        self.direction = self.i2c.readU8(MCP23008_IODIRA)
        self.i2c.write8(MCP23008_GPPUA, 0x00)

    def _transformBit(self, bitmap, bit, value):
        assert value == 1 or value == 0, "Value is %s must be 1 or 0" % value
        if value == 0:
            return bitmap & ~(1 << bit)
        elif value == 1:
            return bitmap | (1 << bit)

    def _changePin(self, port, pin, value, currvalue = None):
        assert pin >= 0 and pin < self.num_gpios, "Pin number %s is invalid, only 0-%s are valid" % (pin, self.num_gpios)
        if not currvalue:
             currvalue = self.i2c.readU8(port)
        newvalue = self._transformBit(currvalue, pin, value)
        self.i2c.write8(port, newvalue)
        return newvalue

    def addPullup(self, pin, value):
        if self.num_gpios <= 8:
            return self._changePin(MCP23008_GPPUA, pin, value)
        else:
            return -1

    # Set pin to input or output mode
    def config(self, pin, mode):
        if self.num_gpios <= 8:
            self.direction = self._changePin(MCP23008_IODIRA, pin, mode)
        else:
            self.direction = -1

        return self.direction

    def output(self, pin, value):
        if self.num_gpios <= 8:
            self.outputvalue = self._changePin(MCP23008_GPIOA, pin, value, self.i2c.readU8(MCP23008_OLATA))
        else:
            self.outputvalue = -1
        return self.outputvalue

    def input(self, pin):
        assert pin >= 0 and pin < self.num_gpios, "Pin number %s is invalid, only 0-%s are valid" % (pin, self.num_gpios)
        assert self.direction & (1 << pin) != 0, "Pin %s not set to input" % pin
        if self.num_gpios <= 8:
            value = self.i2c.readU8(MCP23008_GPIOA)
            return value & (1 << pin)
        else:
            return -1
