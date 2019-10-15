
/*

For correct DS18B20 work you should call a startTempConversion function, 
then wait at least 500ms, then call getTemp function. After that you will be able 
to run getHB and getLB functions witch gives ability to get low and high byte of
temperature from ds18b20.
Also you can use DS18b20_readRam function to read 9 bytes including crc from sensor. 
After calling it you will be able to get actual data from ds18b20_buffer array.

Also you can directly write and read bytes from ds18b20 using read_ds18b20 and write_ds18b20
functions.

*/




void  DS18B20_init(void);
void getTemp();
void DS18b20_readRam(void);
unsigned char read_18b20(void);
void write_18b20(unsigned char data);
void startTempConversion();
char getHB(void);
char getLB(void);

