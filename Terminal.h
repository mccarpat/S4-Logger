#ifndef _LEM_TERMINAL_H
#define _LEM_TERMINAL_H 1

#include <stdio.h>

#ifndef F_CPU
#  error "Define F_CPU before including lemtils/Timer.h"
#endif

/*
 * Terminal.h
 *
 * Created: 09/22/2015
 *  Author: Patrick McCarthy / (Dr. Bossemeyer - GVSU)
 *  Part of Patrick's "lemtils" utility package
 *
 * Purpose:
 *  Facilitate the use of ATmega328P terminal communications / uart
 *  Can now use printf(), scanf(), puts, gets, etc.
 *
 * To Use:
 *  - Paste: #include "lemtils/Terminal.h" // Facilitates terminal communication ( e.g. printf(), scanf(), puts() ) (BAUD 9600). REQUIRES: Terminal_Initialize();
 *  - Must call Terminal_Initialize(); // Sets up USART, stdout, stdin
 *
 * Definitions:
 *  BAUD
 *  MYUBRR
 *  
 * Functions:
 *  - (Uses code from files provided to use in EGR 326, Fall 2015)
 *
 */ 

#define BAUD 9600
#define MYUBRR 1000000/BAUD-1     
//#define MYUBRR F_CPU/16/BAUD-1
// Above works for F_CPU = 16000000UL, but has to be F_CPU/8/BAUD-1 when F_CPU = 8000000UL. So I changed it to 1000000/BAUD-1. 10-11-15


/* -------------------------------------------------------------------------- */
/*
* File provided in EGR 326 - uart.h
* 09-22-15
*/

void USART_Transmit(char c, FILE *stream);
char USART_Receive(FILE *stream);

void USART_Init(unsigned int ubrr);

FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);


void Terminal_Initialize();

// Sets up USART, stdout, stdin
void Terminal_Initialize()
{
	USART_Init(MYUBRR);
	stdout = &uart_output;
	stdin  = &uart_input;
}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*
* File provided in EGR 326 - uart.c
* 09-22-15
*/

#include <avr/io.h>
#include <stdio.h>

void USART_Init(unsigned int ubrr) {
	/* set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;

	UCSR0C = _BV(USBS0) | _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data, 2 stop bits */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void USART_Transmit(char c, FILE *stream) {
	if (c == '\n') {
		USART_Transmit('\r', stream); /* transmit carriage return with new line */
	}
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = c;
}

char USART_Receive(FILE *stream) {
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

/* -------------------------------------------------------------------------- */


// Returns an integer between the integer bounds min and max and echoes it
int Terminal_GetAnInteger(int min, int max)
{
	if (min>=max)
	{   printf("[DEBUG] Terminal_GetAnInteger: min (%i) >= max (%i).\n", min, max);
		return max+1; // Return "max+1" if min>=max
	}
	
	if (max==0) // If max==0, the log10 will break
	{   printf("[DEBUG] Terminal_GetAnInteger: max (%i) equal to zero.\n", max);
		return max+1; // Return "max+1" if min>=max
	}
	
	int myInt;
	while (1)
	{
		int result = scanf("%d", &myInt);
		if (result == EOF) {
			printf("[DEBUG] Terminal_GetAnInteger: Hit EOF.\n");
			continue; // Try that again, also, HEY IT MIGHT GET STUCK HERE
		}
		if (result == 0) {
			while (fgetc(stdin) != '\n'); // Read until a newline is found, may get stuck here.
		}
		
		if ((myInt>max)||(myInt<min)) {
			printf("\nThe entered input of %d is not between the bounds of %i and %i. Try again: ", myInt, min, max);
			continue; // Start this whole loop over
		}
		break; // Satisfied conditions of integer, exit the while loop
		
	}
	printf("%d", myInt);
	return myInt;
}

#endif