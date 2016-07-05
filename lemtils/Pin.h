#ifndef _LEM_PIN_H
#define _LEM_PIN_H 1

#include <avr/io.h>

/*
 * Pin.h
 *
 * Created: 09/28/2015
 *  Author: Patrick McCarthy
 *  Part of Patrick's "lemtils" utility package
 *  
 *  Adapted by Patrick McCarthy from http://www.starlino.com/port_macro.html
 *  Original code credit: ""August 10, 2009 by starlino"" (Modified by Patrick McCarthy)
 *
 * Purpose:
 *  Facilitate the use of pin management on the ATmega328P
 *
 * To Use:
 *  - Paste: #include "lemtils/Pin.h" // Makes managing input/output easier via definitions
 *
 * v1.1 - 10-10-15 - added ability to toggle full ports
 */ 
 
 /*
    BASIC STAMPS STYLE COMMANDS FOR ATMEL GCC-AVR

    Usage Example:
    ———————————————–
    #define pinLed B,5  //define pins like this

    PIN_SET_AS_OUTPUT(pinLED);     //compiles as DDRB |= (1<<5);
    PIN_SET_HIGH(pinLed);         //compiles as PORTB |= (1<<5);
    ———————————————–
*/

// MACROS FOR EASY PIN HANDLING FOR ATMEL GCC-AVR
//these macros are used indirectly by other macros , mainly for string concatenation
#define _SET(type,name,bit)            type ## name  |= _BV(bit)    
#define _CLEAR(type,name,bit)        type ## name  &= ~ _BV(bit)        
#define _TOGGLE(type,name,bit)        type ## name  ^= _BV(bit)    
#define _GET(type,name,bit)            ((type ## name >> bit) &  1)
#define _PUT(type,name,bit,value)    type ## name = ( type ## name & ( ~ _BV(bit)) ) | ( ( 1 & (unsigned char)value ) << bit )

// These macros are used by end user
#define PIN_SET_AS_OUTPUT(pin)            _SET(DDR,pin)    
#define PIN_SET_AS_INPUT(pin)            _CLEAR(DDR,pin)    
#define PIN_SET_HIGH(pin)            _SET(PORT,pin)
#define PIN_SET_LOW(pin)            _CLEAR(PORT,pin)    
#define PIN_TOGGLE(pin)            _TOGGLE(PORT,pin)    
#define PIN_READ(pin)            _GET(PIN,pin)
#define PIN_PULLUP_ON(pin)		_SET(PORT,pin) // <- By Patrick McCarthy
#define PIN_PULLUP_OFF(pin)		_CLEAR(PORT,pin) // <- By Patrick McCarthy

#define PIN_SET_AS_OUTPUT_FULL_B DDRB |= 0xFF
#define PIN_SET_AS_OUTPUT_FULL_C DDRC |= 0xFF
#define PIN_SET_AS_OUTPUT_FULL_D DDRD |= 0xFF
#define PIN_SET_AS_INPUT_FULL_B DDRB &= ~0xFF
#define PIN_SET_AS_INPUT_FULL_C DDRC &= ~0xFF
#define PIN_SET_AS_INPUT_FULL_D DDRD &= ~0xFF
#define PIN_SET_AS_HIGH_FULL_B PORTB |= 0xFF
#define PIN_SET_AS_HIGH_FULL_C PORTC |= 0xFF
#define PIN_SET_AS_HIGH_FULL_D PORTD |= 0xFF
#define PIN_SET_AS_LOW_FULL_B PORTB &= ~0xFF
#define PIN_SET_AS_LOW_FULL_C PORTC &= ~0xFF
#define PIN_SET_AS_LOW_FULL_D PORTD &= ~0xFF
#define PIN_TOGGLE_FULL_B PORTB ^= 0xFF
#define PIN_TOGGLE_FULL_C PORTC ^= 0xFF
#define PIN_TOGGLE_FULL_D PORTD ^= 0xFF

#define PIN_NUMBER_OF(pin)		PIN_GIVEBACK(PORT,pin) // <- By Patrick McCarthy
#define PIN_GIVEBACK(type,name,bit)	type ## bit // <- By Patrick McCarthy
#define PIN_NAMEOF(type,name,bit)	type ## name // <- By Patrick McCarthy

/*
#define MIN(A,B)  (((A)<(B)) ? (A) : (B) )
#define MAX(A,B)  (((A)>(B)) ? (A) : (B) )
#define PUT_IN_RANGE(V,VMIN,VMAX) MAX(VMIN,MIN(VMAX,V))
#define MAP_TO_RANGE(V,VMIN0,VMAX0,VMIN1,VMAX1) ( (VMIN1) +  ( (V) – (VMIN0) ) * ( (VMAX1) – (VMIN1) ) / ( (VMAX0) – (VMIN0) ) )
*/

#endif
