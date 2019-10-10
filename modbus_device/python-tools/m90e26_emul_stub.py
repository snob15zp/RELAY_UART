#!/usr/bin/env python3
import serial

serial_port = '/dev/ttyUSB1'
m90e26_baudrate = 9600

ser = serial.Serial(serial_port, m90e26_baudrate, timeout=1)

try:
    while True:
        if (ser.in_waiting > 0):
            if ser.read() == bytes.fromhex('FE'):
                com_adr = int.from_bytes(ser.read(), 'big')
                com_read = (com_adr & 0x80) > 0
                adr = com_adr & 0x7F

                if com_read:
                    chksum = int.from_bytes(ser.read(), 'big')
                    ser.write(bytes.fromhex('010203'))

                    print('Read. Addr: {} chksum: {}'.format(adr, chksum))
                else:
                    data = int.from_bytes(ser.read(2), 'big')
                    chksum = ser.read()
                    
                    ser.write(chksum)

                    print('Write. Addr: {} Data: {} chksum: {}'.format(adr, data, int.from_bytes(chksum, 'big')))


except KeyboardInterrupt:
    pass


ser.close()

