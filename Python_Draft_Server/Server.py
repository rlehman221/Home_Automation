
# Server.py - TCP socket server to allow devices and interfaces
#  to connect based on custom protocol. Accepted connections are
#  mapped/stored for later execution or opened as a new thread awaiting
#  User input. Conforms to a blocking protocol and should be held on a
#  static IP.
#
#  Date: June-01-2018
#  Created by: Ryan Lehman
# */

import socket
import threading
import select
from queue import Queue
from Keypad import Keypad
from Lock import Lock
from Web_Interface import Web_Interface

NUMBER_OF_THREADS = 2
ADD_WORKER = 1
JOB_NUMBER = [1, 2]
queue = Queue()
socket_queue = Queue()  # Sees which socket to look for
keypad_list = []
lock_list = []
global web_interface


# Creating socket file descriptor
def create_socket():
    try:
        global host
        global port
        global s
        host = ""

        port = 9998
        s = socket.socket()

    except socket.error as msg:
        print("Socket creation error: " + str(msg))


# Binding the socket and listening for connections
def bind_socket():
    try:
        # Attaching socket to the port
        s.bind((host, port))
        s.listen(5)

    except socket.error as msg:
        print("Socket binding error" + str(msg) + "\n" + "Retrying...")
        bind_socket()

def close_connections():
    for keypad in keypad_list:
        keypad.conn.close()

    for lock in lock_list:
        list.conn.closxe()

    del keypad_list[:]
    del lock_list[:]

# Handling connection from multiple clients and saving to a list
# Closing previous connections when server.py file is restarted
def accepting_connections():
    close_connections()

    while True:
        try:
            conn, address = s.accept()
            s.setblocking(1)  # prevents timeout

            try:
                device_type = conn.recv(201480)

                if (str(device_type) == "b'keypad_code'"):
                    my_uid = conn.recv(201480)
                    lock_uid = conn.recv(201480)

                    new_Keypad = Keypad(address, conn, lock_uid, my_uid)
                    keypad_list.append(new_Keypad)
                    queue.put(3)
                    create_workers(ADD_WORKER)

                    print("Keypad has been established :" + address[0])

                elif (str(device_type) == "b'lock_code'"):
                    device_code = conn.recv(201480)
                    my_uid = conn.recv(201480)
                    keypad_uid = conn.recv(201480)
                    new_Lock = Lock(address, conn, device_code, keypad_uid, my_uid)
                    lock_list.append(new_Lock)

                    print("Lock has been established :" + address[0])

                elif (str(device_type) == "b'web_interface'"):
                    global web_interface
                    web_interface = Web_Interface(address, conn)
                    queue.put(5)
                    create_workers(ADD_WORKER)

                    print("Web Interface has been established :" + address[0])

                else:
                    print("Unable to recognize incoming device")
            except:
                print("Something went wrong in obtaining a device code")

        except:
            print("Error accepting connections")

# See all the clients
# Interactive prompt for sending commands
def start_console():
    while True:
        cmd = input('console> ')

        if cmd == 'list':
            list_connections()
        elif 'exit' in cmd:
            break
        else:
            print("Command not recognized")

# Display all current active connections with the client
def list_connections():
    keypad_results = ''

    for i, keypad in enumerate(keypad_list):
        try:
            conn.send(str.encode(' '))
            conn.recv(201480)
        except:
            del keypad_connections[i]
            del keypad_address[i]
            continue

        keypad_results = "Keypad #" + str(i) + " " + str(keypad.ip_addr[0]) + "\n"

    print("---- Keypads----" + "\n" + keypad_results)

    lock_results = ''

    for i, lock in enumerate(lock_list):
        try:
            conn.send(str.encode(' '))
            conn.recv(201480)
        except:
            del lock_connections[i]
            del lock_address[i]
            continue

        lock_results = "Lock #" + str(i) + " " + str(lock.ip_addr[0]) + "\n"

    print("---- Locks----" + "\n" + lock_results)

# Create worker threads
def create_workers(thread_num):
    for _ in range(thread_num):
        t = threading.Thread(target=work)
        t.daemon = True
        t.start()

def keypad_trigger(selected_keypad):
    connected = True
    found_match = False
    inputs = [selected_keypad.conn]
    look_for_uid = selected_keypad.lock_uid
    attached_lock = Keypad("", "", "", "")  # declared as dummy lock
    conn = selected_keypad.conn

    while connected:
        if len(lock_list) == 0:
            # print("their are no locks attached to the server")
            pass
        else:

            for lock in lock_list:
                if (str(lock.my_uid) == str(selected_keypad.lock_uid)):
                    print("Found a match")
                    found_match = True
                    attached_lock = lock

            if (found_match):

                while True:

                    inputData, outputData, errorData = select.select(inputs, inputs, [])
                    data = conn.recv(2096)

                    if len(data) == 0:
                        print("Error: Socket Disconnection")
                        # del lock_connections[index_value]
                        # del lock_address[index_value]
                        connected = False
                        break
                    else:
                        if (str(data) == str(attached_lock.device_code)):
                            print("Access Granted")
                            lock_status = trigger_lock(attached_lock)
                            print(lock_status)
                        else:
                            print("Access Denied")
            else:
                print("Didn't find a lock match")

def trigger_lock(selected_lock):
    conn = selected_lock.conn
    device_code = selected_lock.device_code
    ret_val = 'invalid error'

    try:
        conn.send(str.encode('ready to send'))
        confirmation_code = conn.recv(201480)

        if (str(confirmation_code) == "b'ready to receive'"):
            try:
                print("Got confirmtion code")
                conn.send(str.encode(str(device_code)))
                print("Send device code")
                lock_status = conn.recv(201480)
                print("Got lock status")
                ret_val = lock_status
            except:
                ret_val = 'failed to send unlock code'
        else:
            ret_val = 'lock invalid confirmation code'
    except:
        ret_val = 'lock not ready to receive'

    return ret_val

def web_interface_thread():
    connected = True
    inputs = [web_interface.conn]
    while connected:
        inputData, outputData, errorData = select.select(inputs, inputs, [])
        data = web_interface.conn.recv(2096)

        if len(data) == 0:
            print("Error: Connection Error")
            connected = False
            break
        else:
            print("This is the web data: " + str(data))

# Do next job in the queue
def work():
    while True:
        x = queue.get()
        if x == 1:
            create_socket()
            bind_socket()
            accepting_connections()
        if x == 2:
            start_console()
        if x == 3:
            insert_keypad = keypad_list.pop()
            keypad_trigger(insert_keypad)
        if x == 5:
            web_interface_thread()

        queue.task_done()


def create_jobs():
    for x in JOB_NUMBER:
        queue.put(x)
    queue.join()  # blocks the main thread while the workers process


create_workers(NUMBER_OF_THREADS)
create_jobs()