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
#include <stdint.h>
//#include <time.h>

//Basic bit manipulation macros
//position y of register x
#define SET(x, y) x |= (1 << y)                                    //1
#define CLEAR(x, y) x &= ~(1 << y)                                 //0
#define READ(x, y) ((0x00 == ((x & (1 << y)) >> y)) ? 0x00 : 0x01) //if(1)
#define TOGGLE(x, y) (x ^= (1 << y))                               //inverse

uint32_t millis = 0;
bool IsGGA = false, Time_Set = false, timer_running = false;
; //technicially GA,
int GGA_Index;
char GPS_Data[6]; //HHMMSS
char GPS_Buffer[3];
int time[3];  //HMS
int time2[4]; //HMSMS

ISR(TIMER1_COMPA_vect)
{
    millis++;
    if (time2[3] == 999)
    {
        timeAddSec();
    }
    else
    {
        time2[3]++;
    }
}

ISR(INT0_vect) //PPS
{
    // user code her
    // Diff between GPS_Time and Internal_Time
}

ISR(USART_RX_vect) //GPS transmitts data
{
    char rec_char = UDR0;
    //cli();
    if (GGA_Index > 5) //Time data finished (we need 0..5)
    {
        GGA_Index = 0;
        printf("GPSTime %.6s", GPS_Data);
        convTime(GPS_Data, time);
        //printf(" time: %d %d %d;", time[0], time[1], time[2]);
        printf(" Onboard: %02d %02d %02d %02d;", time2[0], time2[1], time2[2], time2[3]);
        
        if (!timer_running)//Execute only when the first time data is received from the GPS
        {
            SET(TCCR1B, CS11);//TIMER: start timer with prescalar: 8
            timer_running = true;
            time2[0] = time[0];
            time2[1] = time[1];
            time2[2] = time[2]+1; //Very Ugly fix
        }
        printf(" millis = %lu", millis);
        printf("  GPSTime-OnBoardTime = %d s\n", time[2]-time2[2]); //only s for now, better with PPS Trigger
        IsGGA = false;
    }

    if (IsGGA) //checks for GA,
    {
        if (rec_char == ',')
            IsGGA = false;
        GPS_Data[GGA_Index] = rec_char; // write the received char into GPS_Data
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
}

void timeAddSec()
{
    time2[3] = 0;
    (time2[2] == 59) ? timeAddMin() : time2[2]++;
}
void timeAddMin()
{
    time2[2] = 0;
    (time2[1] == 59) ? timeAddH() : time2[1]++;
}
void timeAddH()
{
    time2[1] = 0;
    (time2[0] == 59) ? time[0] = 0 : time2[0]++;
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
    char *uString = malloc(length);
    do
    {
        uString[charlength] = getchar();
        charlength++;
    } while (uString[charlength - 1] != '\n' && charlength < length);
    uString[length] = '\0';
    return uString;
}

void convTime(char *char_array, int *int_array)
{
    for (int i = 0; i < 3; i++)
    {
        int_array[i] = (char_array[i * 2] - 48) * 10 + char_array[i * 2 + 1] - 48;
    }
}

int main()
{
    cli(); //Disable Interrupts
    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
    PCICR = (1 << INT0);
    EICRA = (1 << ISC00) | (1 << ISC01); //Rising Edge Intterupt
    sei();                               //Enable Interrupts
    puts("Hello World!");
    _delay_ms(10);
    SET(TCCR1B, WGM12);  //TIMER: Set CTC Bit, so counter will auto-restart, when it compares true to the timervalue
    OCR1A = 2000;        //TIMER: 16-Bit Value continuesly compared to counter register
    SET(TIMSK1, OCIE1A); //TIMER: Timer/Counter Interrupt Mask Register has to be set to 1 at OCIE0A, so the interrupt will not be masked
    sei();               //TIMER: enable interrupts
    while (1)
    {
        //run temperature compensation
    }
    return 0;
}

//UART Manual
/*
Input:
char *input = uart_getString(length of String);
//do something with input
free(input);

Output:
puts() or printf()
*/

//GPSData
/*
$GPRMC,235316.000,A,4003.9040,N,10512.5792,W,0.09,144.75,141112,,*19
$GPGGA,235317.000,4003.9039,N,10512.5793,W,1,08,1.6,1577.9,M,-20.7,M,,0000*5F
$GPGSA,A,3,22,18,21,06,03,09,24,15,,,,,2.5,1.6,1.9*3E
*/
