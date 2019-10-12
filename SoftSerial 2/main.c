/*
 * main.c - example for using TinySoftSerial for ATtiny
 *
 * Created: 5/8/2018 3:47:42 PM
 * Author : Michael Shimniok
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "SoftwareSerial.h"
#include "ds18b20.h"




#define  F_CPU 8000000L

void initTimer(){
	//Timer period 10mks. Int by OCR0A comp vector.
	TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00);
	TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
	TCNT0=0x00;
	OCR0A=0x9;
	OCR0B=0x00;
	// Timer/Counter 0 Interrupt(s) initialization
	TIMSK0=(0<<OCIE0B) | (1<<OCIE0A) | (0<<TOIE0);
	
}


int main()
{
	initTimer();

	Uart serial0 = {9600,&DDRA,&PORTA,&PINA,PA5,PA1};
	Uart serial1 = {9600,&DDRA,&PORTA,&PINA,PA4,PA0};
	seial_0 = &serial0;
	seial_1 = &serial1;
	softSerialBegin(seial_1);
	softSerialBegin(seial_0);
	
	while (1) {
		startTempConversion();
		_delay_ms(250);
		_delay_ms(250);
		_delay_ms(250);
		getTemp();
		softSerialWrite(getLB(),seial_0);
		softSerialWrite(getHB(),seial_0);
	   if (softSerialAvailable(seial_0)) {
			softSerialWrite( softSerialRead(seial_0),seial_1 );
		}
		
		if (softSerialAvailable(seial_1)) {
			softSerialWrite( softSerialRead(seial_1),seial_0);
		}
	
		}
}



