#ifndef _LEM_EVENTHANDLER_H
#define _LEM_EVENTHANDLER_H 1

/*
 * EventHandler.h
 *
 * Created: 11/27/2015
 *  Author: Patrick McCarthy
 *  Part of Patrick's "lemtils" utility package
 *  A task manager for the ATmega328p, for use with the EGR 326 Alarm Clock project.
 *  Note: This behaves more like a "countdown manager".
 *
 *	To Use:
 *  1. Create an event and initialize it
 *	2. Have a clock that ticks the event every 1ms or whatever you wish
 *  3. Start the event
 *  3. Have an event handler function that checks if the event is "ready" (it has fully counted down)
 *  4. If "ready", do some action (use a switch/case). Restart event if desired.
 *  
 *
 * 
 *  
 *
 */

/* CODE EXAMPLE:
// Lets make a piezo buzz at different tones with a fake square wave with a 1ms timer ticking away at the events.
// 3 events will occur. "PiezoIsReadyToTurnOff" acts as a countdown for the TOTAL length of the piezo buzz (say, 1000ms)
// The PiezoBeepOn and Off activate each other when they fire, and turn the buzzer on and off.
// This creates a square wave on the piezo buzzer that is shut off when PiezoReadyToTurnOff is turns it all off.

bool piezo_is_on = false;
uint8_t piezo_period;
uint8_t piezo_duration;

// Set up your events
struct an_event event_PiezoBeepOn ;
struct an_event event_PiezoBeepOff ;
struct an_event event_PiezoIsReadyToTurnOff ;

// Set up an event ticker
void events_Tick();
void events_Tick(){
	event_Tick(&event_PiezoBeepOn);
	event_Tick(&event_PiezoBeepOff);
	event_Tick(&event_PiezoBlipOff);
	event_Tick(&event_PiezoIsReadyToTurnOff);
}

// Have a timer
volatile unsigned short ms_clock = 0;
#define SCHEDULE_TIME_CEILING       60000

	
//
ISR(TIMER2_COMPA_vect) { //1 msec timer
	ms_clock = ((ms_clock+1) % SCHEDULE_TIME_CEILING); // Investigate if modulo is functioning  try and if instead.
	if (KeypadCooldown > 0) {KeypadCooldown--;}
	events_Tick();
	
	// The handler is in this ISR, but it is best to have it in a while() main loop instead.
	if (event_IsReady(&event_PiezoBeepOn))
	{
		// Turn on the Piezo buzzer if necessary and start the "Off" event
		if(!piezo_is_on){
			PIN_SET_HIGH(PIN_PIEZO);
			piezo_is_on = true;
			event_Start(&event_PiezoBeepOff);
		}
	}
	
	if (event_IsReady(&event_PiezoBeepOff))
	{
		// Turn off the Piezo buzzer if necessary and start the "On" event
		if(piezo_is_on){
			PIN_SET_LOW(PIN_PIEZO);
			piezo_is_on = false;
			event_Start(&event_PiezoBeepOn);
		}
	}
	
	if (event_IsReady(&event_PiezoBlipOff))
	{
		if(piezo_is_on){PIN_SET_LOW(PIN_PIEZO); piezo_is_on = false;}
		//PIN_SET_LOW(PIN_PIEZO); // Turn off the Piezo buzzer
	}
	
	if (event_IsReady(&event_PiezoIsReadyToTurnOff)){
		event_Cancel(&event_PiezoBeepOn);
		event_Cancel(&event_PiezoBeepOff);
		PIN_SET_LOW(PIN_PIEZO);
		piezo_is_on = false;
	}
}

// Beeps the Piezo with a square wave of period for time duration (in ms)
void PiezoBeep(uint8_t period, unsigned short duration);
void PiezoBeep(uint8_t period, unsigned short duration)
{
	event_Initialize(&event_PiezoIsReadyToTurnOff,duration);
	event_Initialize(&event_PiezoBeepOn,(unsigned short)(period/2));
	event_Initialize(&event_PiezoBeepOff,(unsigned short)(period/2));
	event_Start(&event_PiezoIsReadyToTurnOff); // Start a countdown for how long the piezo has been on
	event_StartNow(&event_PiezoBeepOn); // Start the piezo sound immediately
}

int main()
{
		event_Initialize(&event_PiezoBeepOn,1);
		event_Initialize(&event_PiezoBeepOff,PIEZO_BLIP_MS_TIME);
		event_Initialize(&event_PiezoIsReadyToTurnOff,PIEZO_BLIP_MS_TIME);
		
		timer2_ctc(0.001, true); // Set up the 1ms timer to 1 millisecond
		
		while(1)
		{
			// Do stuff
			// May want your event handler here
			if(button_pressed){
				PiezoBeep(6,50); // Beep for 50ms with a square wave of period 6ms (between 2ms and 16ms seem to function, 4 and 6 are best).
				// Can also do a PiezoBeep(CHIRP); if you did a "#define CHIRP 2,20" or something.
			}
		}
}
*/

#include <stdbool.h>

struct an_event {
	volatile unsigned short countdown, default_countdown;
	volatile bool is_planned;
};

void event_Start(struct an_event *event); // Sets the countdown and activates (sets "is_planned" to true)

void event_ResetCountdown(struct an_event *event); // Resets the countdown to default, does not activate/deactivate

void event_StartNow(struct an_event *event); // Activates the event and sets the countdown to 0 (will run on first pass through event handler)

void event_Cancel(struct an_event *event); // Deactivates

void event_Tick(struct an_event *event); // Ticks the event's count down by 1

void event_Initialize(struct an_event *event, unsigned short default_countdown); // Sets the event's default countdown and inactive

bool event_IsReady(struct an_event *event); // Returns true if the count is zero AND the event is active, then deactivates it.

bool event_CountdownIsZero(struct an_event *event); // Returns true if the count is zero, does not deactivate

#endif