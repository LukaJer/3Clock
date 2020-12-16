ISR(USART_RXC_vect)
{
    uint8_t oldsrg = SREG;
    cli();
    char received_char = UDR;

    if (received_char == '$')
    { /* check for '$' */
        GGA_Index = 0;
        CommaCounter = 0;
        IsItGGAString = false;
    }
    else if (IsItGGAString == true)
    { /* if true save GGA info. into buffer */
        if (received_char == ',')
            GGA_Pointers[CommaCounter++] = GGA_Index; /* store instances of ',' in buffer */
        GGA_Buffer[GGA_Index++] = received_char;
    }
    else if (GGA_CODE[0] == 'G' && GGA_CODE[1] == 'G' && GGA_CODE[2] == 'A')
    { /* check for GGA string */
        IsItGGAString = true;
        GGA_CODE[0] = 0;
        GGA_CODE[1] = 0;
        GGA_CODE[2] = 0;
    }
    else
    {
        GGA_CODE[0] = GGA_CODE[1];
        GGA_CODE[1] = GGA_CODE[2];
        GGA_CODE[2] = received_char;
    }
    SREG = oldsrg;
}

//https://www.electronicwings.com/avr-atmega/gps-module-interfacing-with-atmega1632