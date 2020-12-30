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
//#include <time.h> 

bool IsGGA = false, Time_Set=false; //technicially GA,
int GGA_Index;
char GPS_Data[10];
char GPS_Buffer[3];
uint8_t time[8]; //HH:MM:SS:MSMS

ISR(INT0_vect) //PPS
{
    // user code her
    // Diff between GPS_Time and Internal_Time
}

ISR(USART_RX_vect) //GPS transmitts data
{

    char rec_char = UDR0;
    puts(rec_char); // For Debug
    cli();
    if (GGA_Index > 7) //Time data finished
    {
        if (!Time_Set) //Init Time
        {
        strncpy(time,GPS_Data, 8);
        }
        IsGGA = false;
    }
    if (IsGGA) //checks for GA,
    {
        GPS_Data[GGA_Index] = rec_char; // write directly to time?
        GGA_Index++;
    }
    else
    {
        GPS_Buffer[0] = GPS_Buffer[1];
        GPS_Buffer[1] = GPS_Buffer[2];
        GPS_Buffer[2] = rec_char;
        if (GPS_Buffer[0] == 'G' && GPS_Buffer[1] == 'A' && GPS_Buffer[2] == ',')
        {
            IsGGA = true;
            GGA_Index = 0;
            GPS_Buffer[0] = 0;
            GPS_Buffer[1] = 0;
            GPS_Buffer[2] = 0;
        }
    }

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

const char *uart_getString(uint8_t length) //Reads String=Char[length] from UART; returns a pointer
{
    uint8_t charlength = 0;
    char *uString=malloc(length);
    do
    {
        uString[charlength] = getchar();
        charlength++;
    } while (uString[charlength - 1] != '\n' && charlength < length);
    uString[length] = '\0';
    return uString;
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
    sei();                               //Enable Interrupts
    puts("Hello World!");
    _delay_ms(10);
    
    while (1)
    {
        puts("Enter String: ");
        char *input = uart_getString(4);
        printf("You wrote %s\n", input);
        free(input);
    }
    return 0;
}