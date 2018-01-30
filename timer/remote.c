/*	remote.c
 *
 *	Transmit an RF command signal to a remote unit
 *
 *	The protocol used is the software implementation
 *	of the PT2262 Remote Control Encoder chip.
 *
 *	2009	K.W.E. de Lange
 */
#include <util/delay.h>
#include <avr/io.h>
#include "utility.h"


#define	RFPORT	PORTB								// RF transmitter connected to this port
#define	RFBIT	PB0									// RF transmitter input pin
#define	XMBIT	PB1									// RF transmitter power on/off pin

#define DELAY_MICROSECONDS	375						// Time length of a single code bit (in micro-seconds)
#define	REPEATS				4						// Number of times code word is repeated within a code frame

typedef enum { LOW = 0, HIGH, FLOAT } input_t;		// All possible values for a code bit

static input_t bit[12];								// All the bits which make up a code word


/*	Send a signal to a unit
 *
 *	major	major unit id (character A,B,..,P)
 *	minor	minor unit id (integer 1,..,16)
 *	command	ON = 1 or OFF = 0
 *
 */
void sendSignal(uint8_t major, uint8_t minor, uint8_t command)
{
	void sendCodeFrame();
	uint8_t	i;

	major -= 'A';									// major unit id numbers starts at 0
	minor -= 1;										// minor unit id numbers starts at 0

	for (i = 0; i < 4; i++) {
		bit[i] = (major & 0x01) ? FLOAT : LOW;		// bits 0-3 contain major unit id (2^4 = 16 major units)
		major = major >> 1;
	}

	for (i = 4; i < 8; i++) {
		bit[i] = (minor & 0x01) ? FLOAT : LOW;		// bits 4-7 contain minor unit id (2^4 = 16 minor units)
		minor = minor >> 1;
    }

	bit[8] = LOW;									// bits 8-10 contain fixed values
	bit[9] = FLOAT;
	bit[10]= FLOAT;
	bit[11]= (command ? FLOAT : LOW);				// bit 11 contains the command = switch unit on or off

	sendCodeFrame();
}


/*	Send code frame
 *
 *	Frame consists of a code word repeated X times
 *
 */
void sendCodeFrame()
{
	void sendCodeWord();
	uint8_t i;

	bitSet(RFPORT,(1<<XMBIT));						// switch transmitter on
	_delay_ms(2); 									// turn-on delay

	for (i = 0; i < REPEATS; i++)					// repeat code word multiple times
		sendCodeWord();

	bitClr(RFPORT, (1<<XMBIT));						// switch transmitter off
}


/*	Send a 12-bit code word plus single sync bit
 *
 *	Code bits are stored in array bit[]
 *
 */
void sendCodeWord()
{
	void sendCodeBit(input_t);
	void sendSyncBit();
	uint8_t	i;

	for (i = 0; i < 12; i++)						// code word contains 12 bits
		sendCodeBit(bit[i]);

	sendSyncBit();
}


/*	Transmit a single code bit
 *
 *	Format (one period per bit):
 *
 *	Low		1 0 0 0 1 0 0 0
 *	High	1 1 1 0 1 1 1 0
 *	Float	1 0 0 0 1 1 1 0
 */
void sendCodeBit(input_t bit)
{
	switch (bit) {
		case LOW:
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			break;
		case HIGH:
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			break;
		case FLOAT:
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			bitSet(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS * 3);
			bitClr(RFPORT, (1<<RFBIT));
			_delay_us(DELAY_MICROSECONDS);
			break;
	}
}


/*	Transmit synchronization signal
 *
 *	Format (one period per bit)
 *
 *	1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
 *
 * 	(equals 1 x H and 31 x 0)
 */
void sendSyncBit()
{
		bitSet(RFPORT, (1<<RFBIT));
		_delay_us(DELAY_MICROSECONDS);
		bitClr(RFPORT, (1<<RFBIT));
		_delay_us(DELAY_MICROSECONDS * 31);
}
