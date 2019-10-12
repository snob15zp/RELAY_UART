/*
   GccApplication5.c

   Created: 01.10.2019 16:13:38
   Author : User
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define    UART_RX_ENABLED        (1) // Enable UART RX
#define    UART_TX_ENABLED        (1) // Enable UART TX

#define  F_CPU 1000000L

#if defined(UART_TX_ENABLED) && !defined(UART_TX)
# define        UART_TX         PA4 // Use PB3 as TX pin
# define        UART_TX1        PA1  // Use PB3 as TX pin
#endif  /* !UART_TX */

#if defined(UART_RX_ENABLED) && !defined(UART_RX)
# define        UART_RX         PA5 // Use PB4 as RX pin
# define        UART_RX1        PA0 // Use PB4 as RX pin
#endif  /* !UART_RX */

#if (defined(UART_TX_ENABLED) || defined(UART_RX_ENABLED)) && !defined(UART_BAUDRATE)
# define        UART_BAUDRATE   (9600)
#endif  /* !UART_BAUDRATE */

#define    TXDELAY             (int)(((F_CPU/UART_BAUDRATE)-7 +1.5)/3)
#define RXDELAY             (int)(((F_CPU/UART_BAUDRATE)-5 +1.5)/3)
#define RXDELAY2            (int)((RXDELAY*1.5)-2.5)
#define RXROUNDED           (((F_CPU/UART_BAUDRATE)-5 +2)/3)
#if RXROUNDED > 127
# error Low baud rates unsupported - use higher UART_BAUDRATE
#endif


#define DS18B20_PORT PORTA
#define DS18B20_DDR DDRA
#define DS18B20_PIN PINA
#define DS18B20_DQ PA7

volatile char recieveBuffer[8];
volatile char recieveBuffer1[8];
char rxIndex = 0;
char rxIndex1 = 0;
char x;
unsigned char Temp_H, Temp_L, OK_Flag, temp_flag;

// Инициализация DS18B20
unsigned char DS18B20_init(void)
{
  DDRA |= (1 << PA7); // PC0 - выход
  PORTA &= ~(1 << PA7); // Устанавливаем низкий уровень
  _delay_us(490);
  DDRA &= ~(1 << PA7); // PC0 - вход
  _delay_us(68);
  OK_Flag = (PINA & (1 << PA7)); // Ловим импульс присутствия датчика
  // если OK_Flag = 0 датчик подключен, OK_Flag = 1 датчик не подключен
  _delay_us(422);
  return OK_Flag;
}



// Функция чтения байта из DS18B20
unsigned char read_18b20(void)
{
  unsigned char i, data = 0;
  for (i = 0; i < 8; i++)
  {
    DDRA |= (1 << PA7); // PC0 - выход
    _delay_us(2);
    DDRA &= ~(1 << PA7); // PC0 - вход
    _delay_us(4);
    data = data >> 1; // Следующий бит
    if (PINA & (1 << PA7)) data |= 0x80;
    _delay_us(62);
  }
  return data;
}

// Функция записи байта в DS18B20
void write_18b20(unsigned char data)
{
  unsigned char i;
  for (i = 0; i < 8; i++)
  {
    DDRA |= (1 << PA7); // PC0 - выход
    _delay_us(2);
    if (data & 0x01) DDRA &= ~(1 << PA7);
    else DDRA |= (1 << PA7);
    data = data >> 1; // Следующий бит
    _delay_us(62);
    DDRA &= ~(1 << PA7); // PC0 - вход
    _delay_us(2);
  }
}                       // разрешаем прерывания

int main(void)
{
  unsigned int buffer;
  // External Interrupt(s) initialization
  // INT0: Off
  // Interrupt on any change on pins PCINT0-7: On
  // Interrupt on any change on pins PCINT8-11: Off
  MCUCR = (0 << ISC01) | (0 << ISC00);
  PCMSK0 = (0 << PCINT7) | (0 << PCINT6) | (1 << PCINT5) | (0 << PCINT4) | (0 << PCINT3) | (0 << PCINT2) | (0 << PCINT1) | (1 << PCINT0);
  GIMSK = (0 << INT0) | (0 << PCIE1) | (1 << PCIE0);
  GIFR = (0 << INTF0) | (0 << PCIF1) | (1 << PCIF0);

  asm("sei");
  /* Replace with your application code */
  while (1)
  {
    if (recieveBuffer1[0] == '1'&&rxIndex1>7) {
      DS18B20_init();        // Инициализация DS18B20
      write_18b20(0xCC);     // Проверка кода датчика
      write_18b20(0x44);     // Запуск температурного преобразования
      _delay_ms(250);
      _delay_ms(250);
      _delay_ms(250);
      _delay_ms(250);
      DS18B20_init();        // Инициализация DS18B20
      write_18b20(0xCC);     // Проверка кода датчика
      write_18b20(0xBE);     // Считываем содержимое ОЗУ
      Temp_L = read_18b20(); // Читаем первые 2 байта блокнота
      Temp_H = read_18b20();
      uart_putc(Temp_H);
      uart_putc(Temp_L);
      for(char i=0; i<8; i++) recieveBuffer1[i]=0;
	  rxIndex1=0;
    } 
	
	if(recieveBuffer1[0]=='2'&&rxIndex1>7){
		uart_putc(0xFE);
		uart_putc(recieveBuffer1[1]);
		uart_putc(recieveBuffer1[2]);
		uart_putc(recieveBuffer1[3]);
		uart_putc(recieveBuffer1[1]+recieveBuffer1[2]+recieveBuffer1[3]);
				for(char i=0; i<8; i++) recieveBuffer1[i]=0;
				rxIndex1=0;
	} 
	
	if(recieveBuffer1[0]=='3'&&rxIndex1>7){
	uart_putc(0xFE);
	uart_putc(recieveBuffer1[1]);
	uart_putc(recieveBuffer1[1]);
	for(char i=0; i<8; i++) recieveBuffer1[i]=0;
	rxIndex1=0;
}
  }
}

char
uart_getc(void)
{
#ifdef    UART_RX_ENABLED
  char c;
  uint8_t sreg;

  sreg = SREG;
  cli();
  PORTA &= ~(1 << UART_RX);
  DDRA &= ~(1 << UART_RX);
  __asm volatile(
    " ldi r18, %[rxdelay2] \n\t" // 1.5 bit delay
    " ldi %0, 0x80 \n\t" // bit shift counter
    "WaitStart: \n\t"
    " sbic %[uart_port]-2, %[uart_pin] \n\t" // wait for start edge
    " rjmp WaitStart \n\t"
    "RxBit: \n\t"
    // 6 cycle loop + delay - total = 5 + 3*r22
    // delay (3 cycle * r18) -1 and clear carry with subi
    " subi r18, 1 \n\t"
    " brne RxBit \n\t"
    " ldi r18, %[rxdelay] \n\t"
    " sbic %[uart_port]-2, %[uart_pin] \n\t" // check UART PIN
    " sec \n\t"
    " ror %0 \n\t"
    " brcc RxBit \n\t"
    "StopBit: \n\t"
    " dec r18 \n\t"
    " brne StopBit \n\t"
    : "=r" (c)
    : [uart_port] "I" (_SFR_IO_ADDR(PORTA)),
    [uart_pin] "I" (UART_RX),
    [rxdelay] "I" (RXDELAY),
    [rxdelay2] "I" (RXDELAY2)
    : "r0", "r18", "r19"
  );
  asm("sei");
  SREG = sreg;
  return c;
#else
  return (-1);
#endif /* !UART_RX_ENABLED */
}

void
uart_putc(char c)
{
#ifdef    UART_TX_ENABLED
  uint8_t sreg;
  sreg = SREG;
  cli();
  PORTA |= 1 << UART_TX;
  DDRA |= 1 << UART_TX;
  asm volatile(
    " cbi %[uart_port], %[uart_pin] \n\t" // start bit
    " in r0, %[uart_port] \n\t"
    " ldi r30, 3 \n\t" // stop bit + idle state
    " ldi r28, %[txdelay] \n\t"
    "TxLoop: \n\t"
    // 8 cycle loop + delay - total = 7 + 3*r22
    " mov r29, r28 \n\t"
    "TxDelay: \n\t"
    // delay (3 cycle * delayCount) - 1
    " dec r29 \n\t"
    " brne TxDelay \n\t"
    " bst %[ch], 0 \n\t"
    " bld r0, %[uart_pin] \n\t"
    " lsr r30 \n\t"
    " ror %[ch] \n\t"
    " out %[uart_port], r0 \n\t"
    " brne TxLoop \n\t"
    :
    : [uart_port] "I" (_SFR_IO_ADDR(PORTA)),
    [uart_pin] "I" (UART_TX),
    [txdelay] "I" (TXDELAY),
    [ch] "r" (c)
    : "r0", "r28", "r29", "r30"
  );
  SREG = sreg;
  asm("sei");
#endif /* !UART_TX_ENABLED */
}

char uart_getc1(void)
{
#ifdef    UART_RX_ENABLED
  char c;
  uint8_t sreg;
  sreg = SREG;
  cli();
  PORTA &= ~(1 << UART_RX1);
  DDRA &= ~(1 << UART_RX1);
  __asm volatile(
    " ldi r18, %[rxdelay21] \n\t" // 1.5 bit delay
    " ldi %0, 0x80 \n\t" // bit shift counter
    "WaitStart1: \n\t"
    " sbic %[uart_port1]-2, %[uart_pin1] \n\t" // wait for start edge
    " rjmp WaitStart1 \n\t"
    "RxBit1: \n\t"
    // 6 cycle loop + delay - total = 5 + 3*r22
    // delay (3 cycle * r18) -1 and clear carry with subi
    " subi r18, 1 \n\t"
    " brne RxBit1 \n\t"
    " ldi r18, %[rxdelay1] \n\t"
    " sbic %[uart_port1]-2, %[uart_pin1] \n\t" // check UART PIN
    " sec \n\t"
    " ror %0 \n\t"
    " brcc RxBit1 \n\t"
    "StopBit1: \n\t"
    " dec r18 \n\t"
    " brne StopBit1 \n\t"
    : "=r" (c)
    : [uart_port1] "I" (_SFR_IO_ADDR(PORTA)),
    [uart_pin1] "I" (UART_RX1),
    [rxdelay1] "I" (RXDELAY),
    [rxdelay21] "I" (RXDELAY2)
    : "r0", "r18", "r19"
  );
  SREG = sreg;
  asm("sei");
  return c;
#else
  return (-1);
#endif /* !UART_RX_ENABLED */
}

void uart_putc1(char c)
{
#ifdef    UART_TX_ENABLED
  uint8_t sreg;

  sreg = SREG;
  cli();
  PORTA |= 1 << UART_TX1;
  DDRA |= 1 << UART_TX1;
  asm volatile(
    " cbi %[uart_port1], %[uart_pin1] \n\t" // start bit
    " in r0, %[uart_port1] \n\t"
    " ldi r30, 3 \n\t" // stop bit + idle state
    " ldi r28, %[txdelay1] \n\t"
    "TxLoop1: \n\t"
    // 8 cycle loop + delay - total = 7 + 3*r22
    " mov r29, r28 \n\t"
    "TxDelay1: \n\t"
    // delay (3 cycle * delayCount) - 1
    " dec r29 \n\t"
    " brne TxDelay1 \n\t"
    " bst %[ch1], 0 \n\t"
    " bld r0, %[uart_pin1] \n\t"
    " lsr r30 \n\t"
    " ror %[ch1] \n\t"
    " out %[uart_port1], r0 \n\t"
    " brne TxLoop1 \n\t"
    :
    : [uart_port1] "I" (_SFR_IO_ADDR(PORTA)),
    [uart_pin1] "I" (UART_TX1),
    [txdelay1] "I" (TXDELAY),
    [ch1] "r" (c)
    : "r0", "r28", "r29", "r30"
  );
  SREG = sreg;
  asm("sei");
#endif /* !UART_TX_ENABLED */
}



void
uart_puts(const char *s)
{
  while (*s) uart_putc(*(s++));
}

void
uart_puts1(const char *s)
{
  while (*s) uart_putc1(*(s++));
}
ISR(PCINT0_vect) {
  asm("cli");

  if (!(PINA & (1 << PINA0))) { 
    x = uart_getc1();
    recieveBuffer1[rxIndex1++] = x;
  }

  if (!(PINA & (1 << PINA5))) {
  x = uart_getc();
  recieveBuffer[rxIndex++] = x;
  }
  asm("sei");
}


