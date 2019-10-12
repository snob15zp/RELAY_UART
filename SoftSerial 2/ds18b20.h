
/*

For correct DS18B20 work you should call a startTempConversion function, 
then wait at least 500ms, then call getTemp function. After that you will be able 
to run getHB and getLB functions witch gives ability to get low and high byte of
temperature from ds18b20.

Also you can directly write and read bytes from ds18b20 using read_ds18b20 and write_ds18b20
functions.

*/




void  DS18B20_init(void);
void getTemp();
unsigned char read_18b20(void);
void write_18b20(unsigned char data);
void startTempConversion();
char getHB(void);
char getLB(void);

