MCU=atmega328p
CC=avr-gcc
OBJCOPY=avr-objcopy
AVRDUDE=avrdude
CFLAGS=-g -Wall -Os #-mcall-prologues
PRGRM=stk500v2
	
all:
	${CC} -mmcu=${MCU} ${CFLAGS} -o main.o clock.c uart.c
	${OBJCOPY} -j .text -j .data -O ihex main.o main.hex	
	avr-size --mcu=${MCU} -C main.o

flash:
	${AVRDUDE} -p ${MCU} -c ${PRGRM}  -P /dev/ttyUSB0 -U flash:w:main.hex:i -F

clean:
	rm -f *.o *.hex