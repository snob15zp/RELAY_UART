/*
SoftwareSerial.h (formerly NewSoftSerial.h) - 
Single-instance software serial library for ATtiny84A, modified from Arduino SoftwareSerial.
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)
-- Port to ATtiny84A / C by Michael Shimniok (http://www.bot-thoughts.com/)

Notes on the ATtiny84A port. To save space I've:
 - Converted back to C
 - Removed the ability to have mulitple serial ports,
 - Hardcoded the RX pin to PA0 and TX pin to PA1
 - Using & mask versus modulo (%)
 - A few other tweaks to get the code near 1k
More notes:
 - Converted from Arduinoish library stuff (pins etc)
 - Error is too high at 57600 (0.64%) and 115200 (2.12%)
 - Ok at 38400 and lower.
 - Still working out how to prevent missing bits when writing characters

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <avr/io.h>
#include <inttypes.h>
#include <stddef.h>

//
// Hardcoded TX/RX Registers
//
//   For ATtiny85 you MUST use I/O port B.
//
//
// Defines
//
#define true 1
#define false 0
#define HIGH 1
#define LOW 0




typedef volatile struct{
	volatile long _SPEED;
	char* _DDR;
	char* _PORT;
	char* _PIN;
	char _RX_PIN_NUM;
	char _TX_PIN_NUM;
	char _receive_buffer[64];
	uint8_t _receive_buffer_tail;
	uint8_t _receive_buffer_head;
	uint16_t _buffer_overflow;
    uint16_t _rx_delay_intrabit;
	uint16_t _rx_delay_centering;
	uint16_t _rx_delay_stopbit;
	uint16_t _tx_delay;

}Uart;




//
// Types
//
typedef uint8_t bool;
typedef uint8_t byte;


//
// Definitions
//
#define _SS_MAX_RX_BUFF 64 // RX buffer size, must be (1<<n)
#define _SS_RX_BUFF_MASK (_SS_MAX_RX_BUFF-1)
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

//
// public methods
//
Uart *seial_0;
Uart *seial_1;

void softSerialBegin(Uart *p);
void softSerialEnd();
bool softSerialOverflow(Uart *p);
int softSerialPeek(Uart *p);
size_t softSerialWrite(uint8_t byte, Uart *p);
int softSerialRead(Uart *p);
int softSerialAvailable(Uart *p);
void softSerialFlush(Uart *p);
void handler(Uart *p);
char isCalibDataReady();
char getImpulsWidth();


#endif

