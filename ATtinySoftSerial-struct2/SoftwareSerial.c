/*
 SoftwareSerial.c (formerly SoftwareSerial.cpp, formerly NewSoftSerial.c) -
 Single-instance software serial library for ATtiny, modified from Arduino SoftwareSerial.
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



// 
// Includes
// 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "SoftwareSerial.h"





// private static method for timing
static inline void tunedDelay(uint16_t delay);

//
// Lookup table
//
typedef struct _DELAY_TABLE {
	long baud;
	unsigned short rx_delay_centering;
	unsigned short rx_delay_intrabit;
	unsigned short rx_delay_stopbit;
	unsigned short tx_delay;
} DELAY_TABLE;


static const DELAY_TABLE table[] PROGMEM =
{
	//  baud    rxcenter    rxintra    rxstop  tx
	{ 115200,   1,          5,         5,      3,      },
	{ 57600,    1,          15,        15,     13,     },
	{ 38400,    2,          25,        26,     23,     },
	{ 31250,    7,          32,        33,     29,     },
	{ 28800,    11,         35,        35,     32,     },
	{ 19200,    20,         55,        55,     52,     },
	{ 14400,    30,         75,        75,     72,     },
	{ 9600,     50,         114,       114,    112,    },
	{ 4800,     110,        233,       233,    230,    },
	{ 2400,     229,        472,       472,    469,    },
	{ 1200,     467,        948,       948,    945,    },
	{ 600,      948,        1895,      1895,   1890,   },
	{ 300,      1895,       3805,      3805,   3802,   },
};

const int XMIT_START_ADJUSTMENT = 4;


//
// Private methods
//

/* static */
inline void tunedDelay(uint16_t delay) {
	uint8_t tmp = 0;

	asm volatile("sbiw    %0, 0x01 \n\t"
			"ldi %1, 0xFF \n\t"
			"cpi %A0, 0xFF \n\t"
			"cpc %B0, %1 \n\t"
			"brne .-10 \n\t"
			: "+w" (delay), "+a" (tmp)
			: "0" (delay)
	);
}

void handler(Uart *p){
	uint8_t d = 0;
	
	// If RX line is high, then we don't see any start bit
	// so interrupt is probably not for us
	if ( !(*p->_PIN&(1<<p->_RX_PIN_NUM)) ) {
		// Wait approximately 1/2 of a bit width to "center" the sample
		tunedDelay(p->_rx_delay_centering);

		// Read each of the 8 bits
		for (uint8_t i = 0x1; i<10; i++) {
			if(i<9){
				tunedDelay(p->_rx_delay_intrabit-5);
				uint8_t noti = ~i;
				if ((*p->_PIN&(1<<p->_RX_PIN_NUM)))
				d |= i;
				else // else clause added to ensure function timing is ~balanced
				d &= noti;
				PORTB^=1;
					
			} else{
				tunedDelay(p->_rx_delay_stopbit);
				if(!(*p->_PIN&(1<<p->_RX_PIN_NUM))){  //≈—À» Õ≈ œ–»ÿ≈À —“Œœ ¡»“
					FLAG=1;
					pWidth=0;
					TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (1<<CS00); //—“¿–“ “¿…Ã≈–¿ (1ÏÒ)
				}
			}
		} 
			
		

		// skip the stop bit
		//tunedDelay(p->_rx_delay_stopbit);
		
		// if buffer full, set the overflow flag and return
		if (((p->_receive_buffer_tail + 1) & _SS_RX_BUFF_MASK) != p->_receive_buffer_head) {  // circular buffer
			// save new data in buffer: tail points to where byte goes
			p->_receive_buffer[p->_receive_buffer_tail] = d; // save new byte
			p->_receive_buffer_tail = (p->_receive_buffer_tail + 1) & _SS_RX_BUFF_MASK;  // circular buffer
			} else {
			p->_buffer_overflow = true;
		}
	}
}
//
// Interrupt handling, receive routine
//

ISR(PCINT0_vect) {
	if(FLAG){ //—◊»“¿≈Ã ƒÀ»“≈À‹ÕŒ—“‹ »Ãœ”À‹—¿
		if(PINA&(1<<PA4)){
			TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
			TCNT0=0x00;
			FLAG=0;
		}	
	}
	
	if(!(PINA&(1<<PA5))){
		handler(seial_0);
	}
	if(!(PINA&(1<<PA4))){
		handler(seial_1);
	}
	
}

ISR(TIM0_COMPA_vect){
	pWidth++;
}

//
// Public methods
//

void softSerialBegin(Uart *p) {
	
	unsigned i;
	p->_receive_buffer_head = p->_receive_buffer_tail = 0;
	p->_buffer_overflow = false;
	*p->_DDR |= (1<<p->_TX_PIN_NUM); // set TX for output
	*p->_DDR &= ~(1<<p->_RX_PIN_NUM); // set RX for input
	*p->_PORT |= (1<<p->_RX_PIN_NUM)|(1<<p->_TX_PIN_NUM); // assumes no inverse logic
	

	for (i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
		
		long baud = pgm_read_dword(&table[i].baud);
		if (baud == p->_SPEED) {
			p->_rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
			p->_rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
			p->_rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
			p->_tx_delay = pgm_read_word(&table[i].tx_delay);
			// Set up RX interrupts, but only if we have a valid RX baud rate
			GIMSK |= (1<<PCIE0);
			PCMSK0 |= (1<<p->_RX_PIN_NUM);
			tunedDelay(p->_tx_delay);
			
			sei();
			
			return;
		}
	}

	// No valid rate found
	// Indicate an error
}

void softSerialEnd() {
	PCMSK0 = 0;
}

// Read data from buffer
int softSerialRead(Uart *p) {
	// Empty buffer?
	if (p->_receive_buffer_head == p->_receive_buffer_tail)
		return -1;

	// Read from "head"
	uint8_t d = p->_receive_buffer[p->_receive_buffer_head]; // grab next byte
	p->_receive_buffer_head = (p->_receive_buffer_head + 1) & _SS_RX_BUFF_MASK; // circular buffer
	return d;
}

int softSerialAvailable(Uart *p) {
	return (p->_receive_buffer_tail + _SS_MAX_RX_BUFF - p->_receive_buffer_head) & _SS_RX_BUFF_MASK; // circular buffer
}

bool softSerialOverflow(Uart *p) {
	bool ret = p->_buffer_overflow;
	p->_buffer_overflow = false;
	return ret;
}

size_t softSerialWrite(uint8_t b, Uart *p) {
	if (p->_tx_delay == 0) {
		//setWriteError();
		return 0;
	}

	uint8_t oldSREG = SREG; // store interrupt flag
	cli();	// turn off interrupts for a clean txmit

	// Write the start bit
	*p->_PORT &= ~(1<<p->_TX_PIN_NUM); // tx pin low
	tunedDelay(p->_tx_delay + XMIT_START_ADJUSTMENT);

	// Write each of the 8 bits
	for (byte mask = 0x01; mask; mask <<= 1) {
		if (b & mask) // choose bit
			*p->_PORT |= (1<<p->_TX_PIN_NUM); // tx pin high, send 1
		else
			*p->_PORT &= ~(1<<p->_TX_PIN_NUM); // tx pin low, send 0

		tunedDelay(p->_tx_delay);
	}
	*p->_PORT |= (1<<p->_TX_PIN_NUM); // tx pin high, restore pin to natural state

	//sei();
	SREG = oldSREG; // turn interrupts back on
	tunedDelay(p->_tx_delay);

	return 1;
}

void softSerialFlush(Uart *p) {
	uint8_t oldSREG = SREG; // store interrupt flag
	cli();
	p->_receive_buffer_head = p->_receive_buffer_tail = 0;
	SREG = oldSREG; // restore interrupt flag
	//sei();
}


int softSerialPeek(Uart *p) {
	// Empty buffer?
	if (p->_receive_buffer_head == p->_receive_buffer_tail)
		return -1;

	// Read from "head"
	return p->_receive_buffer[p->_receive_buffer_head];
}
