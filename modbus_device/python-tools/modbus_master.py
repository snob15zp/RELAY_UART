#!/usr/bin/env python3
import minimalmodbus
import time
import random

slave_id = 0x10
slave_baudrate = 9600
serial_port = '/dev/ttyUSB0'

instrumentA = minimalmodbus.Instrument(serial_port, slave_id)
instrumentA.serial.baudrate = slave_baudrate
instrumentA.serial.timeout = 1
instrumentA.mode = minimalmodbus.MODE_RTU

def modbus_read(address, instrument, errors=False):
    success = False
    count = 0
    while ((not success) and (count<10)):
        try:
            count += 1
            data = instrument.read_register(address,1)

        except minimalmodbus.SlaveReportedException:
            if errors:
                raise

        except minimalmodbus.NoResponseError:
            if errors:
                raise
            else:
                time.sleep(5)

        else:
            success = True

    if success:
        return data*10
    else:
        return "error"

def modbus_write(address, data, instrument, errors=False):
    success = False
    count = 0
    while ((not success) and (count<10)):
        try:
            count += 1
            instrument.write_register(address, data, functioncode=6)

        except minimalmodbus.SlaveReportedException:
            if errors:
                raise

        except minimalmodbus.NoResponseError:
            if errors:
                raise
            else:
                time.sleep(5)
        else:
            success = True

    if not success:
        print('modbus write error')


print('M90E26')


print(modbus_read(1, instrumentA))
print(modbus_read(2, instrumentA))
print(modbus_read(3, instrumentA))

modbus_write(3, 7531, instrumentA)
print(modbus_read(3, instrumentA))


print('relays')
print(modbus_read(128, instrumentA))
print(modbus_read(129, instrumentA))



print('ds18b20')
print(modbus_read(132, instrumentA))
print(modbus_read(133, instrumentA))



print('debuging')
print('OSCCAL')
print('OSCCAL register value: {}'.format(modbus_read(144, instrumentA)))
print('OSCCAL EEPROM value: {}'.format(modbus_read(145, instrumentA)))
#instrumentA.write_register(145, 165, functioncode=6)
#instrumentA.write_register(145, 255, functioncode=6)
#print('OSCCAL EEPROM value: {}'.format(instrumentA.read_register(145, 1)*10))



print('relays')

for i in range(5):
    modbus_write(128, random.randint(0,1), instrumentA)
    modbus_write(129, random.randint(0,1), instrumentA)
    
    time.sleep(0.2)
    print('{} {}'.format(modbus_read(128, instrumentA),modbus_read(129, instrumentA)))

modbus_write(128, 0, instrumentA)
modbus_write(129, 0, instrumentA)

print('errors')

print('M90E26 read errors: {}'.format(modbus_read(136, instrumentA)))
print('M90E26 write errors: {}'.format(modbus_read(137, instrumentA)))
print('modbus all errors: {}'.format(modbus_read(138, instrumentA)))
print('modbus CRC errors: {}'.format(modbus_read(139, instrumentA)))

