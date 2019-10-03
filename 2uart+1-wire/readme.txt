
Device has 2 UARTS (UART and UART1)

UART master connects to UART1(RX A0, TX A1) , AT90ME26 to UART(RX A5, TX A4).

System operates with fixed data packet size - 8bytes.

If you want to get temperature from ds18b20, uart master have to send command "10000000", in a second master will recieve 2 bytes from DS18B20.

To operate with M90E26 you have 2 commands - read and write.

Write:
Uart master have to send next message: "2 + *register addr* + *msb* + *lsb* + 0000"
Example: "21FF0000" - it will write 0x4646 data to 0x31 register. (Command in ASCII  - '1' = 0x31, 'F'=0x46).
As recieve you will get checksumm from M90E26.

Read:
Uart master have to send next message: "3 + *register addr* + 000000"
Example: "31000000" - it will read 2 bytes from 0x31 (ASCII - '1' = 0x31).
As recieve you will get DATA_MSB + DATA_LSB + CHECKSUMM.

1xxxxxxx - temp
2*addr**msb**lsb*xxxx - write
3*addr*xxxxxx - read