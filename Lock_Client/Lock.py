# Lock.py - TCP socket client that waits for server commands
#  to execute a servo motor.
#
# Date: June-20-2018
# Created by: Ryan Lehman

import socket
import time

# Device Specific Values
DEVICE_CODE = "1997#"
UID = "lock1#"
status = "Locked#"

# Constant startup values
s = socket.socket()
host = '204.48.22.79'
port = 9996

def startOperation():
    global status
    # Wait for confirmation from server
    data = s.recv(20408)
    
    if (data == b'success'):
        while True:
            try:
                data = s.recv(20408)
                # Wait for server status
                if (data == b'ready to receive'):
                    if (status == "Locked#"):
                        status = "Unlocked#"
                    else:
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

setUp()

