#define F_CPU 16000000UL //Set CPU Clock to 16MHz
#define BAUD 9600
#define NTC_pin PB0

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "uart.h"
#include <avr/interrupt.h>
#include <string.h>

ISR(INT0_vect) //PPS
{
    // user code here
    // Diff between GPS_Time and Internal_Time
}

ISR(USART_RX_vect) //GPS transmitts data
{
char rec_char=UDR0;
//TODO GPS Decode and Time Storage

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

void GPS_Time()
{
}

void Internal_Time()
{
}

const char* uart_getString(uint8_t length) //Reads String=Char[length] from UART; returns a pointer
{
    uint8_t charlength = 0;
    char uString[length];
    do
    {
        uString[charlength]=getchar();
        charlength++;
    } while (uString[charlength-1] != '\n' && charlength < length);
    uString[charlength+1] = '\0';
    const char *String=uString;
    return String;
}

int main()
{
    // puts("String") for output
    // char input = getchar(); for input

    cli(); //Disable Interrupts
    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
    PCICR = (1 << INT0);
    EICRA = (1 << ISC00) | (1 << ISC01); //Rising Edge Intterupt
    sei();   //Enable Interrupts
    puts("Hello World!");
    _delay_ms(10);
    const char *input;
    while (1)
    {
      puts("Enter String: ");
      input=uart_getString(4);
      printf("You wrote %s\n", input); 
    }                     
    return 0;
}