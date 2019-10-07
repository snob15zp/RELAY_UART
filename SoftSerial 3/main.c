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
	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: 8000,000 kHz
	// Mode: Normal top=0xFFFF
	// OC1A output: Disconnected
	// OC1B output: Disconnected
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer Period: 8,192 ms
	// Timer1 Overflow Interrupt: Off
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x00;
	OCR1BH=0x00;
	OCR1BL=0x00;
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
	DDRB=0xff;
	while (1) {
		DS18b20_readRam();
		_delay_ms(250);
		_delay_ms(250);
		
		/*softSerialWrite(getLB(),seial_0);
		softSerialWrite(getHB(),seial_0);
	   if (softSerialAvailable(seial_0)) {
			softSerialWrite( softSerialRead(seial_0),seial_1 );
		}
		
		if (softSerialAvailable(seial_1)) {
			softSerialWrite( softSerialRead(seial_1),seial_0);
		}*/
	
		}
}



