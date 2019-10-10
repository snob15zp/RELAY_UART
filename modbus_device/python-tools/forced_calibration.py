#!/usr/bin/env python3
import serial
import time

serial_port = '/dev/ttyUSB0'
baudrate = 9600

start_baudrate = 2400

ser = serial.Serial(serial_port, start_baudrate, timeout=0.5)

ser.write(bytes.fromhex('00'))
print('send force signal')
ser.close()

time.sleep(2)

ser = serial.Serial(serial_port, baudrate, timeout=0.5)
print('Sending')

try:
    while True:
        print('.')
        ser.write(bytes.fromhex('00'))
        answ = int.from_bytes(ser.read(), 'big')
        if answ == 255:
            print('\n Succes')
            break
        else:
            print(answ)
        time.sleep(1.5)

except KeyboardInterrupt:
    pass

ser.close()

