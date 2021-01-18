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
void printDigit2(int digit);
void printDigit4(int digit);
void printDigit(int digit);

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
int BoardTime[4];//{H,M,S,MS}  Time calculated from millis (from 16-Bit timer)
char GPS_Buffer[3];
int GPSTime[3]; //HMS

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
	sei();	
	
    if (setup)
    {
        //start timer with prescalar: 8
        SET(TCCR1B, CS11);

        //start timer with prescalar: 1
        //SET(TCCR1B,CS10);

        setup = false;
    }
    else gps_millis = gps_millis + 1000;
    
	timeAddSec(GPSTime);

    if (counter % 1 == 0)
    {
    	printDigit2(BoardTime[0]);
    	printf(":");
    	printDigit2(BoardTime[1]);
    	printf(":");
    	printDigit2(BoardTime[2]);
    	printf(":");
    	printDigit4(BoardTime[3]);
    	printf(" ");
        printDigit((gps_millis - millis) - delta);//prints +
        printf("%d", (gps_millis - millis) - delta);
        delta = gps_millis - millis;
        
        getTemp();
        printf(" sensor:%u;", ADCW);
        
        printf("\n");
        
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
        //convTime(GPS_Data, GPSTime);
        UCSR0B &= ~(1 << RXCIE0); //Dsiable UART Interrupt
        
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

void getTemp() //Reads and calculates Temperature
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

    return;
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

void convTime(char *char_array, int *int_array)
{
	int i = 0;
    for (; i < 3; i++)
    {
        int_array[i] = (char_array[i * 2] - 48) * 10 + char_array[i * 2 + 1] - 48;
    }
}

void initTimer()
{
    //timer config
    //
    //Set CTC Bit, so counter will auto-restart, when it compares true to the timervalue
    SET(TCCR1B, WGM12);
    //
    //16-Bit Value continuesly compared to counter register
    OCR1A = 1990;//2000 without printf
    //
    //Timer/Counter Interrupt Mask Register has to be set to 1 at OCIE0A, so the interrupt will not be masked
    SET(TIMSK1, OCIE1A);
    //
    //enable interrupts
}

void printDigit(int digit)
{
	if(digit > 0) printf("+");	
}

void printDigit2(int digit)
{
	if(digit < 10) printf("0%d", digit);
	else printf("%d", digit);
}

void printDigit4(int digit)
{
	if(digit < 10) printf("000%d", digit);
	else if(digit < 100) printf("00%d", digit);
	else if(digit < 1000) printf("0%d", digit);
	else printf("%d", digit);
}

int main()
{
    cli(); //Disable Interrupts
    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
    SET(EIMSK,INT0);//set mask
    EICRA = (1 << ISC00) | (1 << ISC01); //Rising Edge Intterupt
    
    initTimer();//setup timer
    
    puts("HH:MM:SS:MSMS Drift");
    _delay_ms(10);
    sei();
    while (1)
    {
        //run temperature compensation
    }
    return 0;
}
