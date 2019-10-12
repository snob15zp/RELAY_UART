#!/usr/bin/env python3
import serial

serial_port = '/dev/ttyUSB0'
m90e26_baudrate = 9600

ser = serial.Serial(serial_port, m90e26_baudrate, timeout=1)

try:

    ser.write(bytes.fromhex('FE8181'))

    data = int.from_bytes(ser.read(2), 'big')
    chksum = int.from_bytes(ser.read(), 'big')

    print('Read. Adr: 0x01 Data: {} chksum: {}'.format(data, chksum))

    ser.write(bytes.fromhex('FE8282'))

    data = int.from_bytes(ser.read(2), 'big')
    chksum = int.from_bytes(ser.read(), 'big')

    print('Read. Adr: 0x02 Data: {} chksum: {}'.format(data, chksum))

    ser.write(bytes.fromhex('FE8383'))

    data = int.from_bytes(ser.read(2), 'big')
    chksum = int.from_bytes(ser.read(), 'big')

    print('Read. Adr: 0x03 Data: {} chksum: {}'.format(data, chksum))

except KeyboardInterrupt:
    pass


ser.close()

