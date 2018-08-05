 # Main.py - TCP socket client that waits for user input
 #  to then send to the server. Using a matrix keypad as
 #  a I/O device along with I2C, higher level methods are
 #  used to read and write to the device.
 #
 # Date: June-08-2018
 # Created by: Ryan Lehman

from Keypad_Controller import keypad
import time
import socket

# Initialize the keypad class
kp = keypad()

# Setup Device Specific variables
LOCK_UID = "lock1"
UID = "keypad1"
s = socket.socket()
host = '10.211.55.5'
port = 9996

def digit():
    # Loop while waiting for a keypress
    retVal = None
    while retVal == None:
        retVal = kp.getValue()
    return retVal

def setup():
    # Connect to designated host and send device specific information
    s.connect((host,port))
    output_str = "keypad_code"
    s.send(str.encode(output_str))
    time.sleep(1)
    s.send(str.encode(UID))
    time.sleep(1)
    s.send(str.encode(LOCK_UID))
    beginOperation()

def beginOperation():

    # Wait for users input then send constructed code to server
    while True:

        try:
            time.sleep(1)
            print("Please enter a 4 digit code: ")

            # Getting first digit
            d1 = digit()
            print(d1)
            # Sleep to allow the next digit press
            time.sleep(1)

            d2 = digit()
            print(d2)
            time.sleep(1)

            d3 = digit()
            print(d3)
            time.sleep(1)

            d4 = digit()
            print(d4)

            # Construct entire code
            user_input = ( str(d1) + str(d2) + str(d3) + str(d4) )
            print("You Entered %s%s%s%s " % (d1, d2, d3, d4))

            # Send code to server
            s.send(str.encode(user_input))
        except:
            print("Problem with keypad connection")
            break

setup()
