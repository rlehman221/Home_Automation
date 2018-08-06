# Lock.py - TCP socket client that waits for server commands
#  to execute a servo motor.
#
# Date: June-20-2018
# Created by: Ryan Lehman

import socket
import time
import RPi.GPIO as GPIO

# Device Specific Values
DEVICE_CODE = "1997#"
UID = "lock1#"
status = "Locked#"
servoPIN = 17
UNLOCKED = 0
LOCKED = 180

# Constant startup values
s = socket.socket()
host = '204.48.22.79'
port = 9996
GPIO.setmode(GPIO.BCM)
GPIO.setup(servoPIN, GPIO.OUT)
p = GPIO.PWM(servoPIN, 50) # GPIO 17 for PWM with 50Hz

def startOperation():
    global status
    # Wait for confirmation from server
    data = s.recv(20408)
    
    if (data == b'success'):
        while True:
            try:
                p.start(0)
                data = s.recv(20408)
                # Wait for server status
                if (data == b'ready to receive'):
                    if (status == "Locked#"):
                        setLock(0)
                        status = "Unlocked#"
                    else:
                        setLock(180)
                        status = "Locked#"
                    s.send(str.encode(status))
                else:
                    print("Server request is invalid")
                    
            except Exception as e:
                print(e)
                return

# Initialize the socket connection
def setUp():
    s.connect((host, port))
    output_str = "lock_device#"
    s.send(str.encode(output_str))
    time.sleep(1)
    s.send(str.encode(UID))
    time.sleep(1)
    s.send(str.encode(DEVICE_CODE))
    time.sleep(1)
    startOperation()

# Uses PWM to set the angle of the servo motor
def SetLock(angle):
   duty = angle / 18 + 2
   p.ChangeDutyCycle(duty)
   time.sleep(1)
   p.ChangeDutyCycle(0)
   p.stop()
   GPIO.cleanup()

setUp()

