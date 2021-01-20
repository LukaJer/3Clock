MCU=atmega328p
CC=avr-gcc
OBJCOPY=avr-objcopy
AVRDUDE=avrdude
CFLAGS=-g -Wall -Os -Wl,-u,vfprintf -lprintf_flt#-mcall-prologues
PRGRM=stk500v2
PORT=/dev/ttyUSB1
	
default:
	${CC} -mmcu=${MCU} ${CFLAGS} -o main.o clock.c uart.c
	${OBJCOPY} -j .text -j .data -O ihex main.o main.hex	
	avr-size --mcu=${MCU} -C main.o

flash:
	${AVRDUDE} -p ${MCU} -c ${PRGRM}  -P ${PORT} -U flash:w:main.hex:i -F

flasha:
	${AVRDUDE} -c arduino -b 57600 -P ${PORT} -p ${MCU} -vv -U flash:w:blink.hex 

clean:
	rm -f *.o *.hex
	
all:
	${CC} -mmcu=${MCU} ${CFLAGS} -o main.o clock.c uart.c
	${OBJCOPY} -j .text -j .data -O ihex main.o main.hex	
	avr-size --mcu=${MCU} -C main.o
	${AVRDUDE} -p ${MCU} -c ${PRGRM}  -P ${PORT} -U flash:w:main.hex:i -F