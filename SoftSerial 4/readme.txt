Program designed on atmel studio 7.0, tested on proteus 8.
Supports next features:
  - Software uart(s)
  - DS18B20 connection
  - Measurement of calibration impulse width and future calibration by it. (accuracy is  +0.325us maximum*)
For correct work of software uart interface you should manually configure
external pin interrupt vector. As example, if you configured uart, and it has RX pin on PA0
you have manually check - if the interrupt is caused by PA0. Then, if yes, pass the uart structure to the handling function.

Software uart is half-duplex, it means that you can't receive and transmit data simultaneously.
DS18B20 interface have to be configured in definitions which  located on ds18b20 interface header file. 


*Accuracy rate is tested on simulation in Proteus 8 at 8MHz.

