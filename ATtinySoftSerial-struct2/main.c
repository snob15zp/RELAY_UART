/*
 * main.c - example for using TinySoftSerial for ATtiny
 *
 * Created: 5/8/2018 3:47:42 PM
 * Author : Michael Shimniok
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "SoftwareSerial.h"

// incoming buffer
volatile char *inbuf[32];
// outgoing buffer
volatile char *outbuf[32];

#define  F_CPU 8000000L




int main()
{

	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 125,000 kHz
	// Mode: CTC top=OCR0A
	// OC0A output: Disconnected
	// OC0B output: Disconnected
	// Timer Period: 1 ms
	TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00);
	TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
	//TCCR0B=(0<<WGM02) | (0<<CS02) | (1<<CS01) | (1<<CS00);
	TCNT0=0x00;
	OCR0A=0x7C;
	OCR0B=0x00;
	// Timer/Counter 0 Interrupt(s) initialization
	TIMSK0=(0<<OCIE0B) | (1<<OCIE0A) | (0<<TOIE0);
	
	Uart serial0 = {9600,&DDRA,&PORTA,&PINA,PA5,PA1};
	Uart serial1 = {9600,&DDRA,&PORTA,&PINA,PA4,PA0};
	seial_0 = &serial0;
	seial_1 = &serial1;
	softSerialBegin(seial_1);
	softSerialBegin(seial_0);
	DDRB=0xff;
	
	
	

	while (1) {
	   if (softSerialAvailable(seial_0)) {
			softSerialWrite( softSerialRead(seial_0),seial_1 );
		}
		
		if (softSerialAvailable(seial_1)) {
			softSerialWrite( softSerialRead(seial_1),seial_0);
		}
		_delay_ms(1);
	 
		}
}

