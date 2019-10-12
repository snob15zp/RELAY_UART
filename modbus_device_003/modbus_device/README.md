## Folders
`atmel-studio-6.2` - sources and project in Atmel Studio 6.2 of attiny84a frimware
`bin` - compiled firmware from atmel studio
`bin/ext 8MHz crystal` - for device with external crystal 8 MHz used
`bin/int RC osc` - for device with internal RC oscillator used
`avr-gcc` - sources of attiny84a frimware
`python-tools` - tools for debug and calibration int. RC osc.

## Description

In the firmware implemented:
* two software serial ports
* modbus rtu device protocol, listening on first software serial
* IC M90E26 uart protocol (master). IC connected to second software serial
* one wire protocol for reading DS18B20 temperature sensor
* controling two relays
* calibration of internal RC oscillator

Device listening for modbus rtu commands. Device appeals to necessary devices (M90E26, DS18B20, relays) or internal memory cells only when it got proper command from modbus rtu.

It is necessary to make int. RC oscillator calibration after flasing device.
It is recomended make int. RC oscillator calibration if device (attiny85a) send wrong answers to mudbus requests.

## Known issues

Device may work unstable and freezes on internal RC oscillator.

## Build firmware for external crystal oscillator

Comment out in `settings.h` lines:
`#define OSCCAL_CALIBRATION			1`
`#define OSCCAL_FORCE_CALIBRATION	1`

Build firmware and upload it to MCU.
Proper crystal is 8 MHz.

## Build

In Atmel Studio.

`Build -> Build solution` or F7

## Flashing frimware

On Atmel Studio.

https://www.microchip.com/webdoc/GUID-ECD8A826-B1DA-44FC-BE0B-5A53418A47BD/index.html?GUID-6D2437A8-62FE-4621-843C-E2B7FCC36486

## Flashing fuses

How to setup fuses from Atlmel Studio:
https://www.youtube.com/watch?v=iXOP_36dqwU

### For int. RC. osc

Low : `0xE2`

### For external crystal

Low : `0xEE`

## Int. RC osc. calibration

### In Atmel Studio
Write calibration constant to EEPROM 0x0FF.

Official Atmel Studio documentation:
https://www.microchip.com/webdoc/GUID-ECD8A826-B1DA-44FC-BE0B-5A53418A47BD/index.html?GUID-6D2437A8-62FE-4621-843C-E2B7FCC36486

Tools -> Device Programming 
Select your programmer. Target device: ATtiny84A, ISP. Press `Apply`
Select `Oscillator Callibration`. Calibrate for frequency: 8MHz.
Press `Read`.
Set `Address` to `0x0FF`. Press write.

### Auto calibration on first start by serial.

It works when firmware was built with `OSCCAL_CALIBRATION` definition in `setting.h`.
At first start when EEPROM was erased. Or 0xFF was wroten to EEPROM adress 0x0FF.
Device wait for calibration packets on first serial. 9600 8N1. It's need to send one byte `0x00`. Delay for 2 seconds. Maximum of attempts is 255.
Calibration finished with success if device send answer with value `0xFF`.

From PC you can use `callibration.py` from `python-tools`

### Recalibration by serial.

It works when firmware was built with `OSCCAL_FORCE_CALIBRATION` definition in `setting.h`.
Send to device null packet on baud less than 9600.
Further, everything happens similarly to paragraph `Auto calibration on first start by serial.`

From PC you can use `force_callibration.py` from `python-tools`

## Testing

Tested with `AVR ISP mkii` programmer, ATtiny84A, M90E26, DS18B20 and led's in minimal configuration.
See `python-tools/README.md` for software testing instructions.

