#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

#ifndef VREF
#  error "Define VREF before including lemtils/ADC.h"
#endif

/*
 * ADC.h
 *
 * Created: 09/22/2015
 *  Author: Patrick McCarthy
 *  Part of Patrick's "lemtils" utility package
 *
 * Purpose:
 *  Facilitate the use of the ATmega328P analog to digital converter (ADC)
 *  Uses AVcc as VREF
 *
 * To Use:
 *  - Paste: #include "lemtils/ADC.h" // Includes functions for using the ADC. REQUIRES: ADC_Initialize(); ADC_SetAsInput(pin);
 *  - Can use something like "#define PHOTOCELL 0" and then ADC_Value(PHOTOCELL) to get value from PC0.
 *  - First use ADC_SetAsInput(PHOTOCELL) though to set that pin PCx as an input.
 *  - Can get values from pins PC0 through PC5, pass 0-5 to ADC_Value(pin).
 *
 * Pins Used:
 *  DDRC - Whichever pin is called to be checked on DDRC
 *
 * Definitions:
 *  VREF 5 // <- 5V operating voltage (will need to change this if using 3.3V)
 *  
 * Functions:
 *  ADC_Initialize(); // Initializes the ADC (ADLAR, Voltage reference is AVcc, Prescaler 128, enables ADC)
 *  uint8_t ADC_Value(uint8_t pin); // Returns the ADC measured value at PCx (x = PIN) as an 8-bit value [0-255]
 *  ADC_SetAsInput(uint8_t pin); // Sets the pin (PCx) as an input for use with the ADC
 *  int ADC_Millivolts(uint8_t pin); // Returns the ADC measured value in millivolts at PCx (x = PIN) as an int (with respect to VREF)
 *  double ADC_Volts(uint8_t pin); // Returns the ADC measured value in Volts at PCx (x = PIN) as an int (with respect to VREF)
 *  + more...
 */ 


int ADC_Millivolts(uint8_t pin);
double ADC_Volts(uint8_t pin);
void ADC_Initialize();
uint8_t ADC_Value(uint8_t pin);
void ADC_SetAsInput(uint8_t pin);

// Initializes the ADC (ADLAR, Voltage reference is AVcc, Prescaler 128, enables ADC)
void ADC_Initialize()
{
	ADMUX |= _BV(ADLAR); // ADCH register now contains the 8 MSB
	ADMUX |= _BV(REFS0); // Voltage Reference Selection is internal AVcc (VREF)
	ADCSRA |= (_BV(ADPS1) | _BV(ADPS1) | _BV(ADPS0)); // Set ADC clock with prescale 128
	ADCSRA |= _BV(ADEN); // Enables the ADC
}


// Sets the pin (PCx) as an input for use with the ADC
void ADC_SetAsInput(uint8_t pin)
{
	DDRC &= ~_BV(pin);
}


// Returns the ADC measured value at PCx (x = PIN) as an 8-bit value [0-255]
uint8_t ADC_Value(uint8_t pin)
{
	ADMUX &= 0xf0; // Sets MUX3-MUX0 to 0, resetting what pin our ADC reads
	ADMUX |= pin; // Turns on MUX2:0 appropriately for reading from PC0-PC5
	
	ADCSRA |= _BV(ADSC); // ADC Start Conversion
	
	while ( (ADCSRA & _BV(ADSC)) ); // Waits while ADSC is 1 (Conversion still occurring)
	
	return ADCH;
	/* Returns the ADCH register value (between 0 and 255) which is our read value
	 * 
	 * COULD set this function as a uint16_t and return the full ADC to get values between 0 and 1023.
	 * Example code:
	 *	int Vin;
	 *	Vin = (Reading * VREF * 3.90625); // Vin is now in millivolts (3.9 mV per bit? 1/256 (for V) * 1000 (mV / V) = 3.9)
	 *	printf("Voltage: %d mV\n", Vin);  // Will print Vin in millivolts. Cannot printf a float with present Atmel Studio without linking library things.
	 *	printf("Voltage: %d.%.3d V\n\n", (int) (Vin/1000), (int) (Vin%1000)); // A bit of a trick to printing "X.XXX V".
	 */
}


int ADC_Millivolts(uint8_t pin) // Returns the ADC measured value in millivolts at PCx (x = PIN) as an int (with respect to VREF)
{
  // Vin = (Reading * VREF * 3.90625) because Vin is now in millivolts (3.9 mV per bit? 1/256 (for V) * 1000 (mV / V) = 3.9)
  // (double)((1/256)*1000) yields 3.90625 mV, as in mV per division, where ADC_Value yields 256 divisions
  // Below is basically   x/256  *  VREF, where x is  between 0 and 255
  return (int)( ADC_Value(pin) * VREF * (double)(1000/256) );
}


double ADC_Volts(uint8_t pin) // Returns the ADC measured value in Volts at PCx (x = PIN) as an int (with respect to VREF)
{
	return (double)( ADC_Value(pin) * VREF * (1/256) );
}

#endif