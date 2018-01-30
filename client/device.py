# device.py
#
# Setup the serial connection
# Send commands to - and receive data from the timer device.

import datetime
import struct
from collections import namedtuple

import serial

device_info_type = namedtuple("device_info_type", "hw_version sw_version memory_size action_count")
action_type = namedtuple("action_type", "valid major minor dd mm yy wd hh mn cmd")
datetime_info_type = namedtuple("datetime_info_type", "date time weekday")

port = None


def connect(comport):
    global port

    try:
        port = serial.Serial(port=comport, baudrate=9600,
                             parity=serial.PARITY_NONE, bytesize=serial.EIGHTBITS, stopbits=serial.STOPBITS_ONE)
        if not port.isOpen():
            port.open()
            port.isOpen()
    except serial.SerialException as e:
        port = None
        print("SerialException:", e)
        return False
    return True


def close():
    global port
    if port is not None:
        port.close()
        port = None


def read(size=1):
    return port.read(size)


def write(data):
    return port.write(data)


# Start the timer
# Send:     'A'
# Receive:  '0' or '1'
#
def start_timer():
    write(b"A")
    r = read(1)
    return ord(r[0:1])


# Stop the timer
# Send:     'B'
# Receive:  '0' or '1'
#
def stop_timer():
    write(b"B")
    r = read(1)
    return ord(r[0:1])


# Read date and time from the device
# Send:     'C'
# Receive:  7 bytes - dd, mm, yy, weekday, hh, mm, ss
#
def get_datetime():
    write(b"C")
    b = read(7)
    dd = ord(b[0:1])
    mm = ord(b[1:2])
    yy = ord(b[2:3])
    wd = ord(b[3:4])
    hh = ord(b[4:5])
    mn = ord(b[5:6])
    ss = ord(b[6:7])

    date = datetime.date(2000 + yy, mm, dd)
    time = datetime.time(hh, mn, ss)

    return datetime_info_type(date, time, wd)


# Set data and time on the device
# Send:     'D'
#           7 bytes: dd, mm, yy, weekday, hh, mm, ss
# Receive:  '0' or '1'
#
def set_datetime():
    now = datetime.datetime.now()

    b = bytearray()

    b.append(now.day)
    b.append(now.month)
    b.append(now.year - 2000)
    b.append(now.isoweekday())  # monday = 1
    b.append(now.hour)
    b.append(now.minute)
    b.append(now.second)

    write(b"D")
    write(b)

    r = read(1)

    return ord(r[0:1])


# Read an action from the timer device
# Send:     'E'
#           2 byte integer for action number
# Receive:  10 bytes - valid, (char)major, minor, dd, mm, yy, wd, hh, mm, cmd
#           translated into an action_type tuple
#
def get_action(index):
    write(b"E")

    # H = unsigned short (2 bytes)

    b = struct.pack("H", index)
    write(b)

    r = read(10)

    # Unpack binary data, B = unsigned char, c = char

    return action_type._make(struct.unpack("BcBBBBBBBB", r))


# Send an action to the timer device
# Send:     'F'
#           10 bytes - valid, (char)major, minor, dd, mm, yy, wd, hh, mm, cmd (all read from an action_type tuple)
# Receive:  '0' or '1'
#
def set_action(index, action):
    write(b"F")

    # H = unsigned short (2 bytes)

    b = struct.pack("H", index)
    write(b)

    # Pack binary date, B = unsigned character

    b = struct.pack("BBBBBBBBBB", action.valid, ord(action.major), action.minor, action.dd, action.mm, action.yy,
                    action.wd, action.hh, action.mn, action.cmd)
    write(b)

    r = read(1)

    return ord(r[0:1])


# Switch a unit on or off
# Send:     'G'
#           3 bytes - major, minor, cmd
# Receive:  '0' or '1'
#
def switch(major, minor, command):
    write(b"G")

    b = struct.pack("BBB", major, minor, command)
    write(b)

    r = read(1)

    return ord(r[0:1])


# Read device info
# Send:     'H'
# Receive:  24 bytes
#
def get_info():
    write(b"H")
    b = read(24)

    hw_version = b[0:3].decode().lstrip()
    sw_version = b[3:6].decode().lstrip()
    memory_size_string = b[6:11].decode()
    memory_size = 0 if len(memory_size_string) == 0 else int(memory_size_string)
    action_count_string = b[11:16].decode()
    action_count = 0 if len(action_count_string) == 0 else int(action_count_string)

    return device_info_type(hw_version, sw_version, memory_size, action_count)


# Write device info
# Send:     'I'
#           8 bytes
#
def set_info(hw_version, memory_type, memory_size):
    # to avoid accidental use ...
    if 1:
        return

    write(b"I")

    b = struct.pack("BB", hw_version, memory_type)
    write(b)
    b = struct.pack("IH", memory_size, 0xABFE)
    write(b)

    # Note: no OK or Error response from device


# Print debugging output to the serial port
# Send:     'J'
# Receive:  '0' or '1'
#
def set_verbose():
    write(b"J")

    b = read(1)

    return ord(b[0:1])


# Force a reset (currently waits for the user the press the reset button
# Send:     'K'
#
def reset():
    write(b"K")
