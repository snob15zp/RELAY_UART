# Preparation

## Install Python 3.5.2 and above.

## Install pacakges from requirments.txt
You can use VirtualEnv or others.

# Usage

## Check your M90E26 IC.
Connect a M90E26 bread board to USB UART or RS-232 to TTL converter. And connect it to power supply.

Change 
`serial_port = '/dev/ttyUSB0'`
in `m90e26_test.py` to actual serial port, which connected to the M90E26 IC.

For MS Windows it's need only `'COMn'`. For example:
`serial_port = 'COM1'`

Run `m90e26_test.py`

In case of success, data from the address `0x03` should be different from zeros.

## Check attiny84 bread borad with m90e26 emulator(stub).
Connet two USB UARTs or RS-232 to TTL converters to the attiny84 bread borad.
Connect a power supply to the attiny84 bread borad.

Change `serial_port` in `m90e26_emul_stub.py` and `modbus_master.py` to actual serial ports.
Run `m90e26_emul_stub.py`. 
Run `modbus_master.py`. 

Check that the `modbus_master.py` script exits without exceptions (errors).
Stop `m90e26_emul_stub.py` by CTRL-C.

## Check attiny84 bread borad with m90e26.
Connect a M90E26 bread board to the attiny84 bread borad.
Connect a power supply to the attiny84 bread borad.

Change `serial_port` in `modbus_master.py` to actual serial port.
Run `modbus_master.py`. 

Check that the `modbus_master.py` script exits without exceptions (errors).

## First calibration of internal RC oscillator
Change `serial_port` in `calibration.py`. 
Run `calibration.py`.
Wait for `Success.`

## Force calibration of internal RC oscillator
Change `serial_port` in `force_calibration.py`. 
Run `force_calibration.py`.
Wait for `Success.`
