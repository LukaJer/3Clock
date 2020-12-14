# 3Clock
Clock using temperatrure-compensated internal Oszillator, GPS and DCF77  
TDB: 
- which uC  
- details  

Accurate Arduino Nano/ATmega328 clock by compensating for temperature drift of the oscillator. The idea is to observe the clock speed at different temperatures with an oscilloscope and getting a formula for calculating drift per °C. This formula will be used by the uC to adjust the timer value needed for a 0.1s or 1s interrupt.
It will be written in C.
