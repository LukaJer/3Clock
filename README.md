# 3Clock
Clock using temperatrure-compensated internal Oscillator, GPS (and DCF77)  

Accurate Arduino Nano/ATmega328 clock by compensating for temperature drift of the oscillator. The idea is to observe the clock speed at different temperatures with an oscilloscope and getting a formula for calculating drift per °C. This formula will be used by the uC to adjust the timer value needed for the 0.1s or 1s interrupt. The compensated clock will then be compared to GPS-Time and its PPS interrupt.

TODO:
* Accuracy (s or ms)
* Time Stroage (how to handle overflows)
  * GPS_Data char or int
* Temperature Read
* Temperature Compensate

Notes:
* Onboard Arduino Nano Serial (CH340) can be used for Serial IO (Debugging)


Commands to use:
* build/flash:
  * disconnect RX0 before flash
  * make = compile
  * make flash = flash via ISP
  * make flasha = flash vie Onboard CH340 (not working)
  * make all = make && make flash
* screen /dev/ttyUSB0
* ls /dev/ | grep USB or ls /dev/ttyUSB*
* git
  * pull
* git
  * add [filename]
  * commit
  * push


