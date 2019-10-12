#include <avr/io.h>
#include "ds18b20.h"
#include <avr/delay.h>
unsigned char Temp_H, Temp_L, OK_Flag;

void  DS18B20_init(void)
{
  DDRA |= (1 << PA7); // PC0 - выход
  PORTA &= ~(1 << PA7); // Устанавливаем низкий уровень
  _delay_us(490);
  DDRA &= ~(1 << PA7); // PC0 - вход
  _delay_us(68);
  OK_Flag = (PINA & (1 << PA7)); // Ловим импульс присутствия датчика
  // если OK_Flag = 0 датчик подключен, OK_Flag = 1 датчик не подключен
  _delay_us(422);
  
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


void startTempConversion(){
	DS18B20_init();        // Инициализация DS18B20
    write_18b20(0xCC);     // Проверка кода датчика
    write_18b20(0x44);     // Запуск температурного преобразования
}

void getTemp(){
	  DS18B20_init();        // Инициализация DS18B20
      write_18b20(0xCC);     // Проверка кода датчика
      write_18b20(0xBE);     // Считываем содержимое ОЗУ
      Temp_L = read_18b20(); // Читаем первые 2 байта блокнота
      Temp_H = read_18b20();
	
}

char getLB(void){
	return Temp_L;
}
char getHB(void){
	return Temp_H;
}