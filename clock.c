#define F_CPU 16000000UL //Set CPU Clock to 16MHz
#define BAUD 9600
#define NTC_pin PB0

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "uart.h"



double getTemp() //REads and calculates Temperature
{
    //ADMUX = (ADMUX & ~(0x1F)) | (0 & 0x1F); //A0 as input Needed?
    ADCSRA |= (1 << ADSC);                  // Read ADC
    while (ADCSRA & (1 << ADSC))            // Convert and wait
    {
    } 
    //TODO: ADC to Temp conversion
    /*
    ADCW*NTC_Coeff
    */

   return 0; //
}

void initADC()
{
    ADMUX = (1 << REFS1) | (0 << REFS0);                 // Set internal as Vref
    ADMUX |= NTC_pin;                                        // Set A0 as input
    ADCSRA = (1 << ADPS1) | (1 << ADPS0) | (1 << ADPS0); //Frequenxy prescaler 128 (CLK/125kHz)
    ADCSRA |= (1 << ADEN);                               // Enable ADC
    ADCSRA |= (1 << ADSC);                               // Read ADC
    //Dummy Readout seems to be good practice
    while (ADCSRA & (1 << ADSC))                         // Convert and wait
    {
    }
    (void)ADCW;                                          //Dump Dummy ADCW
}

int main()
{
    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;
// puts("String") for output
// char input = getchar(); for input

    return 0;
}