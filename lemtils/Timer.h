#ifndef _LEM_TIMER_H
#define _LEM_TIMER_H 1

#include <stdio.h>
#include <avr/io.h>
#include <stdbool.h>

#ifndef F_CPU
#  error "Define F_CPU before including lemtils/Timer.h"
#endif

#ifdef NDEBUG
#  warning "Debugging features disabled: NDEBUG is defined."
#endif

/*
 * Timer.h
 *
 * Created: 09/22/2015
 *  Author: Patrick McCarthy
 *  Part of Patrick's "lemtils" utility package
 *  Heavily modified from skeletal code from CptSpaceToaster and AetheralLabyrint
 *
 * Purpose:
 *  Facilitate the use of the ATmega328P on-board timer(s)
 *
 * To Use:
 *  - Paste: #include "lemtils/Timer.h" // Includes functions for using the on-board timer. Allows delay(ms). REQUIRES: Timer_Timer1_Initialize(); (Or similar)
 *  
 * Functions:
 *  Timer_Timer1_Initialize(); // Initializes Timer1 with a prescaler of 256 (Range is up to 1048.56 ms)
 *  delay(unsigned ms); // Delays run in milliseconds. Range is up to 1048.56 ms assuming a 1:256 prescaler. (Up to ~1 second)
 *  MORE - Fix
 *  IMPLEMENT THIS http://playground.arduino.cc/Main/TimerPWMCheatsheet
 *             and https://arduino-info.wikispaces.com/Timers-Arduino
 *
 *
 * Note from EGR 226 for Timer 1: (Assumes F_CPU = 16MHZ)
 *   PS 1     Resolution (us/tick) 0.0625    Range (ms) 4.09594
 *   PS 8     Resolution (us/tick) 0.5       Range (ms) 32.7675
 *   PS 64    Resolution (us/tick) 4         Range (ms) 262.140
 *   PS 256   Resolution (us/tick) 16        Range (ms) 1048.560
 *   PS 1024  Resolution (us/tick) 64        Range (ms) 4194.240
 *
 *  Resolution = PS/(F_CPU=16MHz) * 10^6(us)/1s
 *  Range = Res[us/tick]*65535[ticks]
 *
 */ 


bool _WAS_TIMER1_INITIALIZED = false;
int Timer_Timer1_Prescaler_Compare(uint16_t Ton, uint16_t period, uint8_t prescaler, uint8_t output_pin, bool enable_compare_A, bool enable_compare_B);
int Timer_Timer1_PWM_Set_Compare(double Time_On, double period, uint8_t output_pin, bool enable_compare_A, bool enable_compare_B);
int Timer_Timer1_PWM(double Time_On, double period, uint8_t output_pin);
int Timer_Timer1_PWM_DC(double period, uint8_t DutyCycle, uint8_t output_pin);

void Timer_Timer1_Initialize(); // Initializes Timer1 with a prescaler of 256 (Range is up to 1048.56 ms)
void delay(unsigned ms);


// Delays run in milliseconds. Range is up to 1048.56 ms assuming a 1:256 prescaler. (Up to ~1 second)
void delay(unsigned ms) {
	unsigned cycles = ms * 62.5; //Assumes 16 MHz clock, 1:256 prescaler
	TCNT1 = 0;
	OCR1A = cycles;
	TIFR1 = _BV(OCF1A); // Set-To-Clear bit
	while((TIFR1 & _BV(OCF1A)) == 0)  ;
}


// Initializes Timer1 with a prescaler of 256 (Range is up to 1048.56 ms)
void Timer_Timer1_Initialize (void) // Initialize the timer
{
	_WAS_TIMER1_INITIALIZED = true;
	TCCR1A = 0;
	TCCR1B = _BV(CS12); //Sets PS to 256, "0,1,0".
	//printf("\n[DEBUG] Timer1 Initialized with PS of 256.");
}


// ---------- Start trying to use the below code, more modular once developed ------------
void timer1_init(void)
{
	_WAS_TIMER1_INITIALIZED = true;
	DDRB |= _BV(2); // Set PB2 as output. Will need to change this whole thing if you want PB1.
	TCCR1A = 0; //Set timer
	// Phase correct PWM mode is WGM "1011"
	
	// PS of 1 has range 4.09594 ms. <- max period
	// PS of 8 has range 32.7675 ms. <- max period
	TCCR1A = _BV(WGM11) | _BV(WGM10); // PWM Phase Correct 10 Bit, Update OCR1x at TOP, TOV1 Flag set at BOTTOM
	
	// Use PB2. Need COM1A1 if want PB1, and set THAT as an output.
	// Set OC1A/OC1B on Compare Match
	TCCR1A |= (_BV(COM1B1) | _BV(COM1B0));
	TCCR1B = _BV(WGM13) | _BV(CS11);
	TCNT1 = 0; //Start at zero
}


void pwm_PB2 (double Ton, double period)
{
	if(_WAS_TIMER1_INITIALIZED==false){
			timer1_init();
			//printf("\n[DEBUG] [WARNING] Timer 1 not initialized, doing it for you.");
		}

	//Example of calculating:
	// 20 ms / 32.768 ms = X / 65535, X = 40000 for a 20ms period
	// This method is applied below for both the period and Ton
	//double range = 4.09594;
	double range = 32.7675;
	unsigned int max_count = 65535;
	OCR1A = ((int)((period/range)*max_count));
	OCR1B = ((int)((Ton/range)*max_count));
}


// Enables Timer1 in PWM mode, arguments are in seconds and PBx (x = pin)
int Timer_Timer1_PWM(double Time_On, double period, uint8_t output_pin)
{
	return Timer_Timer1_PWM_Set_Compare(Time_On, period, output_pin, false, false);
}


// Enables Timer1 in PWM mode, arguments are in seconds and PBx (x = pin)
int Timer_Timer1_PWM_Set_Compare(double Time_On, double period, uint8_t output_pin, bool enable_compare_A, bool enable_compare_B)
{
	/*  PWM Diagram by Patrick McCarthy 09-28-15:
	
	           ______                   ______
		      /      \                 /      \
		     /        \               /        \
		  ---          ---------------          -----
		     |<- Ton ->|<-   Toff  ->|
			 | <-       Period    -> |
			 
	    Timer1 in WGM 11 (phase-correct PWM) mode, TCNT1 counts up, hits OCR1A, back down.
		COM1B1 set to 1, COM1B0 set to 0, PB2 will do the following:
		
						^ TCNT1 (Max 65535 = 0xFFFF)
						|
						|
			OCR1A ->	| _ _ _ _ _ _ _ _ _    
						|      /\        /\
						|     /  \      /  \      .<- TCNT1
			OCR1B ->	|_ _ /_ _ \ _ _/_ _ \_ _ /
						|   /|    |\  /|    |\  /|
				   0 -> |__/_|____|_\/_|____|_\/_|________
				          | |<-Tclk  | <- . -> |
							 |    |    |  ^ = 2*OCR1A = Period
							 |    |    |    |    |
				PB2 ->	_____      ____      ____
						     |    |    |    |    |
							 |____|    |____|    |____
							                     ^<- Clear PB2 on match (TCNT1==OCR1B) and counting UP
											^<- Set PB2 on match (TCNT==OCR1B) and counting DOWN
											
			Period of the waveform is 2*OCR1A.
			Duty Cycle is the number of cycles for which PB2 is high.
			DC = Ton/period = OCR1B/OCR1A
			
			To stop the PWM waveform, set COM1B1 to 0: PB2 becomes disconnected from timer hardware.
			
			Note: In WGM #10 (WGM13 & WGM11), it operates like above but with the TOP at ICR1.
	*/
	
	// Note, the function could also take a bool for enabling compare modes if you add that
	if (Time_On < 0 || period < 0){
		printf("\n[DEBUG] Cannot initialize Timer1: Time_On < 0 or Period < 0.");
		return 1; // Returns 1 if error
	}
	
	uint32_t cycles_on = Time_On*(F_CPU);
	uint32_t cycles_period = period*(F_CPU);
	
	if (cycles_period > (uint32_t)1024 * 65535) { // 1024 accounts for prescaler, set later
		printf("\n[DEBUG] Period exceeded maximum Timer1 threshold.");
		return 1;
	}
	if (Time_On > period) {
		printf("\n[DEBUG] Time_On is greater than Period.");
		return 1;
	}
	
	// Calculate prescaler
	uint8_t prescaler=1;
	while(cycles_period>65535) {
		cycles_on/=prescaler>2?4:8;
		cycles_period/=prescaler>2?4:8;
		prescaler++;
	} // Count prescaler up in powers of 2 until the period is under 65535 cycles. Sets the cycles_on and cycles_period appropriately.
	
	//printf("\n[DEBUG] [INFO] The prescaler for Timer1 has been set to %d, given a period of %f.", prescaler, period);
	/* KEEP THIS, IT IS USEFUL FOR DEBUGGING */
	//printf("[HEY] Timer 1 Set: PS = %" PRIu8 "    cycles_on = %" PRIu32 "     cycles_period = %" PRIu32 "\n", prescaler, cycles_on, cycles_period);

	// Recast cycles_on and cycles_period to 16 bit and pass them on
	// Pass on output compares here if you add it
	return Timer_Timer1_Prescaler_Compare((uint16_t)cycles_on, (uint16_t)cycles_period, prescaler, output_pin, enable_compare_A, enable_compare_B);
}


// Enables Timer1 in PWM mode, arguments are in seconds, percent (eg 10, 50), and PBx (x = pin)
int Timer_Timer1_PWM_DC(double period, uint8_t DutyCycle, uint8_t output_pin)
{
	if (period < 0 || DutyCycle < 0 || DutyCycle > 100){
		//printf("\n[DEBUG] Cannot initialize Timer1: Period < 0 or 0 > DutyCycle > 100.");
		return 1; // Returns 1 if error
	}
	
	return Timer_Timer1_PWM_Set_Compare(((double)((double)DutyCycle*period/100)), period, output_pin, false, false);
}


// Ton=Time_On, and period, are both uint16_t in CYCLES not seconds
int Timer_Timer1_Prescaler_Compare(uint16_t Ton, uint16_t period, uint8_t prescaler, uint8_t output_pin, bool enable_compare_A, bool enable_compare_B)
{
	// Sets up Timer1 in WGM #10 (top is ICR1)
	//if(_WAS_TIMER1_INITIALIZED==false){
	//	timer1_init();
	//	log_warn("Timer 1 not initialized, doing it for you.")
	//}
	if (output_pin < 0) {
		// No output pin selected
	} else if (output_pin == 1) {
		DDRB |= _BV(output_pin); // Set output_pin to an OUTPUT
		OCR1A = Ton >> 1; // Up then down, so divide by 2 for accurate response (?)
	} else if (output_pin == 2) {
		DDRB |= _BV(output_pin); // Set output_pin to an OUTPUT
		OCR1B = Ton >> 1;
	} else {
		//printf("\n[DEBUG] [ERROR in Timer setup] Output pin must be 1, 2, or below 0");
		return 1;
	}
	
	// Used to toggle OCF1A/OCF1A (in TIFR1) on TCNT1 compare matches (used for interrupts when I get there)
	TIMSK1 = _BV(OCIE1A)*enable_compare_A | _BV(OCIE1B)*enable_compare_B;
	
	// We are counting up AND down, so divide the period in half and set that as our top (ICR1)
	// Since 2*Top==period, set ICR1 = period/2
	ICR1 = period >> 1; 
	
	// Allow Timer 1 to control PB1 or PB2, set timer to PWM mode with ceiling at ICR1
	TCCR1A = _BV(COM1A1)*(output_pin==1) | _BV(COM1B1)*(output_pin==2) | _BV(WGM11);
	// Start the timer with the given prescaler, finish setting timer to PWM mode with ceiling at OCR1A
	TCCR1B	= _BV(WGM13) | prescaler;
	_WAS_TIMER1_INITIALIZED = true;
	return 0;
}


/* THIS CODE IS DIRECTLY FROM CPTSPACETOASTER   11-6-15 */
/* Note that this is the inspiration for the above code, and as I understand the above, I feel confident utilizing this function without modification */
/* Takes in a period in seconds, and sets timer2 to the closest possible value always rounded down */
int timer2_ctc(double period, bool enable_compare_A) {
	if (period < 0) {
		//printf("Error: Time was set to a value below 0\n");
		return 1;
	}
	uint32_t cycles = (period*(F_CPU));
	if (cycles > (uint32_t)1024 * 255) {
		//printf("Error: Value exceeded maximum timer2 threshold\n");
		return 1;
	}
	// Enable Interrupts
	TIMSK2 = _BV(OCIE2A)*enable_compare_A;
	// Set the Timer Mode to "Clear Timer on Compare Match (CTC)"
	TCCR2A = _BV(WGM21);
	// Calculate prescaler
	uint8_t prescaler=1;
	uint8_t prescales[] = {8,4,2,2,2,4};
	while(cycles>255) {
		cycles/=prescales[prescaler-1];
		prescaler++;
	}
	// Set cycle limit
	OCR2A = cycles;
	// Set prescaler
	TCCR2B = prescaler;
	// reset the counter
	TCNT2 = 0;
	return 0;
}

#endif