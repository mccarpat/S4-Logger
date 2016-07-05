#ifndef _LEM_EVENTHANDLER_C
#define _LEM_EVENTHANDLER_C 1

/*
 * EventHandler.c
 *
 * Created: 11/27/2015
 *  Author: Patrick McCarthy
 *  Part of Patrick's "lemtils" utility package
 *  A task manager for the ATmega328p, for use with the EGR 326 Alarm Clock project.
 *
 */

#include "EventHandler.h"

void event_Start(struct an_event *event){
	event->countdown = event->default_countdown; // Means: event.countdown = event.default_countdown;
	event->is_planned = true;
}


void event_StartNow(struct an_event *event){
	event->countdown = 0;
	event->is_planned = true;
}


void event_ResetCountdown(struct an_event *event){
	event->countdown = event->default_countdown;
}


void event_Cancel(struct an_event *event){
	event->is_planned = false;
}


void event_Tick(struct an_event *event){
	if ((event->is_planned == true) && (event->countdown > 0)) {event->countdown--;}
}


void event_Initialize(struct an_event *event, unsigned short default_countdown){
	event->default_countdown = default_countdown;
	event->is_planned = false;
}


bool event_IsReady(struct an_event *event)
{
	if ((event->is_planned == true) && (event->countdown == 0)){
		event->is_planned = false; // Turn off the event countdown
		return true;
		} else {
		return false;
	}
}

bool event_CountdownIsZero(struct an_event *event)
{
	if (event->countdown == 0){
		return true;
		} else {
		return false;
	}
}

#endif