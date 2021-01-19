//NOTE: please use avr-gcc options to make printf float work:
// -Wl,-u,vfprintf -lprintf_flt

#define F_CPU 16000000UL //Set CPU Clock to 16MHz
#define BAUD 115200
#define NTC_pin PC0

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

void convTime(char *char_array, int *int_array);
void timeAddH();
void timeAddSec();
void timeAddMin();
void initTimer();
int ADCRead();
float getTemp();

int Ro = 100, B = 3974; //Nominal resistance 100K, Beta constant
int Rseries = 100;      // Series resistor 100K
float To = 298.15;      //nominal temperature, 25degC

uint32_t millis = 0;
uint32_t millis_ISR = 0;
uint32_t gps_millis = 0;
uint32_t delta = 0;
bool setup = true;
int counter = 0;
int BoardTime[4];
bool IsGGA = false;
int GGA_Index, timeDiff;
char GPS_Data[6]; //HHMMSS
int BoardTime[4]; //{H,M,S,MS}  Time calculated from millis (from 16-Bit timer)
char GPS_Buffer[3];
int GPSTime[4]; //HMS



ISR(TIMER1_COMPA_vect)
{
    millis++;
    if (BoardTime[3] == 999)
    {
        timeAddSec(BoardTime);
    }
    else
    {
        BoardTime[3]++;
    }
}

ISR(INT0_vect) //PPS
{
    sei(); //interrupts get automatically disabled when ISR is activated. We want our millis still be updated! This fixes the problem of inconsistent timing caused by printf :))

    if (setup)
    {
        //start timer with prescalar: 8
        SET(TCCR1B, CS11);

        //start timer with prescalar: 1
        //SET(TCCR1B,CS10);

        setup = false;
    }
    else
        gps_millis = gps_millis + 1000;

    timeAddSec(GPSTime);

    if (counter % 1 == 0)
    {
        //main print output every 1s (PPS)
        printf("%02d:%02d:%02d:%04d",BoardTime[0],BoardTime[1],BoardTime[2],BoardTime[3]);

        int value = ADCRead();
        printf(" %d", value);
        float temp = getTemp(value);
        printf(" %.1d°C", (double)temp);

        printf(" %ld\n", (gps_millis - millis) - delta);
        delta = gps_millis - millis;
    }
    counter++;
}

ISR(USART_RX_vect) //GPS transmitts data
{
    char rec_char = UDR0;
    //cli();
    if (GGA_Index > 5) //Time data finished (we need 0..5)
    {
        GGA_Index = 0;
        //printf("GPSTime %.6s", GPS_Data);
        convTime(GPS_Data, BoardTime);
        convTime(GPS_Data, GPSTime);
        UCSR0B &= ~(1 << RXCIE0); //Disable UART Interrupt
    }
    if (IsGGA) //checks for GA,
    {
        if (rec_char == ',')
            IsGGA = false;
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
}

void timeAddSec(int *Time)
{
    Time[3] = 0;
    (Time[2] == 59) ? timeAddMin() : Time[2]++;
}
void timeAddMin(int *Time)
{
    Time[2] = 0;
    (Time[1] == 59) ? timeAddH() : Time[1]++;
}
void timeAddH(int *Time)
{
    Time[1] = 0;
    (Time[0] == 23) ? GPSTime[0] = 0 : Time[0]++;
}

void convTime(char *char_array, int *int_array)
{
    
    for (int i = 0; i < 3; i++)
    {
        int_array[i] = (char_array[i * 2] - 48) * 10 + char_array[i * 2 + 1] - 48;
    }
}

int ADCRead()
{

    //connect to A5 : ADMUX |= (1<<MUX2) | (1<<MUX2)
    SET(ADMUX, MUX2); 
    SET(ADMUX, MUX0);

    //start conversion : ADCSRA |= (1<<ADSC )
    SET(ADCSRA, ADSC);

    //wait while Bit ADSC of Register ADCSRA is enabled
    while (ADCSRA & (1 << ADSC))
        ;

    //return two bytes
    return (int)(ADC);
}

float getTemp(int reading)
{
    //Read analog outputof NTC module, i.e the voltage across the thermistor
    float Vi = ((float)reading) * (5.0 / 1023.0);
    //Convert voltage measured to resistance value
    //All Resistance are in kilo ohms.
    float R = (Vi * Rseries) / (5 - Vi);
    /*Use R value in steinhart and hart equation
	Calculate temperature value in kelvin*/
    float T = 1 / ((1 / To) + ((log(R / Ro)) / B));
    float Tc = T - 273.15; // Converting kelvin to celsius
    return Tc;
}

void initADC()
{
    ADMUX = (1 << REFS0); // set VRef
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //Enable ADC & set prescaler to 128
}

void initTimer()
{
    //timer config
    //
    //Set CTC Bit, so counter will auto-restart, when it compares true to the timervalue
    SET(TCCR1B, WGM12);
    //
    //16-Bit Value continuesly compared to counter register
    OCR1A = 1990; //2000 without printf
    //
    //Timer/Counter Interrupt Mask Register has to be set to 1 at OCIE0A, so the interrupt will not be masked
    SET(TIMSK1, OCIE1A);
    //
    //enable interrupts
}

int main()
{
    cli(); //Disable Interrupts
    uart_init();
    initTimer(); //setup timer
    initADC();   //setup ADC

    stdout = &uart_output;
    stdin = &uart_input;
    SET(EIMSK, INT0);                    //set mask
    EICRA = (1 << ISC00) | (1 << ISC01); //Rising Edge Intterupt

    puts("HH:MM:SS:MSMS ADC TMP    Drift");
    _delay_ms(10);
    sei();
    while (1)
    {
        //run temperature compensation
    }
    return 0;
}
