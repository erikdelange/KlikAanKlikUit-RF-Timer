/*	timer1.c
 *
 *	Routines for setting up 16-bit timer1
 *
 *	2009	K.W.E. de Lange
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#define	INITTICK	30

typedef enum { FALSE = 0, TRUE} boolean;

extern volatile boolean timerWakeup;
extern volatile boolean timerEnable;
extern volatile long disableTimeOut;


void timer1Init(void)
{
	TCCR1B = (1<<WGM12)|(1<<CS12)|(1<<CS10);		// CTC mode, prescaler 1024
	OCR1A  = 0x9895;								// compare match interrupt will occur every 2 seconds
	TIMSK1 = (1<<OCIE1A);							// enable compare match interrupt

	timerWakeup = FALSE;
}


/*	Timer 1 interrupt handler
 *
 *	Sets variable 'timerWakeup' to TRUE every 60 seconds
 *
 */
ISR(TIMER1_COMPA_vect)								// timer1 interrupt is triggered every 2 seconds
{
	static uint8_t tick = INITTICK;

	if (--tick == 0) {								// wait for 2 x 30 seconds = 60 seconds
		tick = INITTICK;
		timerWakeup = TRUE;							// then signal it is OK to start processing actions
		if (timerEnable == FALSE)					// if the timer was disabled ... 
			if (!disableTimeOut--)					// ... for more then X minutes (set to 10) then ...
				timerEnable = TRUE;					// ... automatically enable the timer again
	}
}
