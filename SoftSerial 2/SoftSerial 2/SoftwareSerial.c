/*
BASED ON: https://github.com/shimniok/ATtinySoftSerial 
For: Attiny84A 8 MHz

Added: 
-Support of multiple UARTS.
-Measurement of long impulse width (longer than selected baud rate byte).

For correct work you have to manually configure PCINT interrupts. As example if receiver or uart are on PA0 you have to detect low level
when interrupt occurs and call a function handler() with a Uart struct created before for this interface before as param.  

Measured width stored on pWidth variable and can be taken by calling getImpulsWidth. To check if data about width is ready you can call
isCalibDataReady function and it will return 1 if data ready.

This code configures:

-Timer 0, CHANNEL A COMPARE INTERRUPT.
-PCINT interrupts.

*/



// 
// Includes
// 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "SoftwareSerial.h"


uint8_t FLAG;
uint16_t pWidth;
uint8_t calibTimeReady;


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

ISR(TIM0_COMPA_vect){
	pWidth++; //Ticks every 10 mks 
}
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
		for (uint8_t i = 0x1; i; i <<= 1) {
			tunedDelay(p->_rx_delay_intrabit-1);
			uint8_t noti = ~i;
			if ((*p->_PIN&(1<<p->_RX_PIN_NUM)))
			d |= i;
			else // else clause added to ensure function timing is ~balanced
			d &= noti;
		};
		tunedDelay(p->_rx_delay_stopbit-5);
		
		if(!(*p->_PIN&(1<<p->_RX_PIN_NUM))){ //If no stop bit - run timer and measure calibration  impulse width.
			pWidth=0;
			TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00); //rum timer(10���)
			FLAG=1;
			
		}
		
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
	if(FLAG){ //if flag of measurement present - wait for front to end of measuerment. 
		if(PINA&(1<<PA4)){
			TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
			TCNT0=0x00;
			FLAG=0;
			calibTimeReady=1;
		}
	}
	
	if(!(PINA&(1<<PA5))){ 
		handler(seial_0);
	}
	if(!(PINA&(1<<PA4))){  //If low level on UART RX channel detected - call handler function with uart port as argument.
		handler(seial_1);
	}
	
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


char isCalibDataReady(){
	if(calibTimeReady){
		return 1;
	} else return 0;
}

char getImpulsWidth(){
	char w = pWidth;
	calibTimeReady=0;
	return w;
}