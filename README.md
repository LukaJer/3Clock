# 3Clock
Clock using temperatrure-compensated internal Oscillator, GPS (and DCF77)  

Accurate Arduino Nano/ATmega328 clock by compensating for temperature drift of the oscillator. The idea is to observe the clock speed at different temperatures by computing the time-delta to GPS Time and getting a formula for calculating drift per °C. This formula will be used by the µC to adjust the timer value needed for the 0.1s or 1s interrupt.

TODO:
- [x] Accuracy (s or ms)
- [x] Time Stroage (how to handle overflows)
  - [x] GPS_Data char or int
- [x] Temperature Read
- [x] Temperature Compensate

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

possible roadmap:
- [ ] volatile for asynchronus variable change  
- [ ] make BoardTime main Clock e.g. for output (already in testing branch)
