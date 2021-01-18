#define F_CPU 16000000UL //Set CPU Clock to 16MHz
#define BAUD 115200
#define NTC_pin PB0

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "uart.h"
#include <avr/interrupt.h>
#include <string.h>
//#include <time.h>

//standard integer definitions (ie, uint8_t, int32_t, etc)
#include <stdint.h>




//Basic bit manipulation macros
//position y of register x
#define SET(x,y) x |= (1 << y) //1
#define CLEAR(x,y) x &= ~(1<< y) //0
#define READ(x,y) ((0x00 == ((x & (1<<y))>> y))?0x00:0x01) //if(1)
#define TOGGLE(x,y) (x ^= (1<<y)) //inverse

uint32_t millis = 0;
uint32_t millis_ISR = 0;
uint32_t gps_millis = 0;
uint32_t delta = 0;
bool setup = true;
int counter = 0;

ISR(TIMER1_COMPA_vect)
{
	millis++;
}

ISR(INT0_vect) //PPS
{
	if(setup)
	{
		//start timer with prescalar: 8
		SET(TCCR1B,CS11);
		
		//start timer with prescalar: 1
		//SET(TCCR1B,CS10);
		
		setup = false;
	}
	else gps_millis = gps_millis + 1000;
	
	if(counter%30 == 0)
	{
		printf("%ld\n", (gps_millis - millis) - delta);
		delta = gps_millis - millis;
	}
	counter++;
}

double getTemp() //Reads and calculates Temperature
{
    //ADC is 10bit
    /*
    Needed  or already set in initADC()?
    ADMUX = (ADMUX & ~(0x1F)) | (0 & 0x1F); //A0 as input 
    */
    ADCSRA |= (1 << ADSC);       // Read ADC
    while (ADCSRA & (1 << ADSC)) // Convert and wait
    {
    }
    /*
    measurement is stored as (unit16_t) ADCW
    */
    //TODO: ADC to Temp conversion

    return 0;
}

void initADC()
{
    ADMUX = (1 << REFS1) | (0 << REFS0);                 // Set internal as Vref
    ADMUX |= NTC_pin;                                    // Set A0 as input
    ADCSRA = (1 << ADPS1) | (1 << ADPS0) | (1 << ADPS0); //Frequenxy prescaler 128 (CLK/125kHz)
    ADCSRA |= (1 << ADEN);                               // Enable ADC
    ADCSRA |= (1 << ADSC);                               // Read ADC
    //Dummy Readout seems to be good practice
    while (ADCSRA & (1 << ADSC)) // Convert and wait
    {
    }
    (void)ADCW; //Dump Dummy ADCW
}

int main()
{
    cli(); //Disable Interrupts
    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
    
    //SET EXTERNAL INTERRUPT
    //
   	//Rising Edge Intterupt
    SET(EICRA,ISC00);
    SET(EICRA,ISC01);
    //
    //activate INTO external interrupt
    SET(EIMSK,INT0);
    
    
    puts("Hello World!");
    _delay_ms(10);
    
    
    //timer config
    //
    //Set CTC Bit, so counter will auto-restart, when it compares true to the timervalue
	SET(TCCR1B,WGM12);
	//
	//16-Bit Value continuesly compared to counter register
	OCR1A = 2000;
	//
	//Timer/Counter Interrupt Mask Register has to be set to 1 at OCIE0A, so the interrupt will not be masked
	SET(TIMSK1,OCIE1A);
	//
	//enable interrupts
	sei();
	

    while (1)
    {
        //run temperature compensation
    }
    return 0;
}
