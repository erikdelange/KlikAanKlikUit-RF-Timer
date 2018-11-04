/*	main.c
 *
 *	Stand-alone timer for KlikAanKlikUit remote controlled plug-in sockets
 *
 *	2009	K.W.E. de Lange
 */
#include <stdio.h>
#include <stdint.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "define.h"
#include "usart0.h"
#include "ds1307.h"
#include "24cXX.h"
#include "remote.h"
#include "utility.h"


/*	Definitions and functions to enable printf output to the serial port.
 *
 */
static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,_FDEV_SETUP_WRITE);

static int uart_putchar(char c, FILE *stream)
{
	putch(c);

	return c;
}


/*	Automatically disable the watchdog after system reset.
 *
 *	After a system reset the watchdog timer is set by default to 15 mS. Beware that it is not disabled automatically.
 *	Only after a power-on the timer is stopped.
 *
 */
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));	// .init3 is run after stack initialization

void wdt_init(void)
{
	mcusr_mirror = MCUSR;				// save source of reset + clear the watchdog reset flag
	MCUSR = 0;							// disable the watchdog timer
	wdt_disable();
}


typedef struct							// timer action record layout
{
	uint8_t valid;						// <> 0 if entry is valid, else 0
	uint8_t	major;						// major unit id, characters 'A' to 'P'
	uint8_t	minor;						// minor unit id, integers 1 to 16
	uint8_t	dd;							// day fraction of date, or 0 in case of no date
	uint8_t	mm;							// month fraction of date, or 0 in case of no date
	uint8_t yy;							// year fraction of date, or 0 in case of no date
	uint8_t day;						// weekday number (ISO numbering, Monday = 1), or 0 in case of no weekday
	uint8_t	hrs;						// hour fraction of time when to execute
	uint8_t	min;						// minute fraction of time when to execute
	uint8_t	cmd;						// switch off if cmd == 0 else switch on
} ACTION;

typedef struct							// hardware info record layout
{
	uint8_t	version;					// hardware version
	uint8_t	memoryType;					// type of EEPROM: 0 = AVR EEPROM, 1 = I2C EEPROM
	uint32_t memorySize;				// memory size in bytes
} HARDWARE;


HARDWARE hardware;						// hardware information

int maxActionIndex;						// the maximum number of actions which can be stored in EEPROM
volatile boolean timerWakeup;			// flag indicating whether the timer must wake up
volatile boolean timerEnable;			// flag to enable or disable the timer, regardless of wakeup
volatile long disableTimeOut;			// counter to arrange automatic reset of timerEnable to TRUE
boolean verbose = FALSE;				// flag indicating whether debugging output must be sent to the client terminal


typedef enum
{
	INIT,
	NORMAL
} run_t;								// ..


void timer1Init(void);
void parse(void);
int getTime(void);
int setTime(void);
int setAction(void);
int getAction(void);
int switchUnit(void);
int getInfo(void);
int	setInfo(void);
int	countActions();
void initActions(void);
void checkActions(run_t type);
void executeActions(int from, int to, int dd, int mm, int yy, int day, run_t type);
int	readAction(int index, ACTION *action);
int	writeAction(int index, ACTION *action);


int main(void)
{
	stdout = &mystdout;										// for use of printf

	/*	Hardware initialization
	 *
	 */
	MCUSR = 0;
	wdt_disable();

	usart0Init();
	timer1Init();
	i2cInit();

	/*	Load and process timer device information
	 *
	 */
	DS1307ReadData(0x08, sizeof(HARDWARE), (uint8_t *)&hardware);
	maxActionIndex = (hardware.memorySize / sizeof(ACTION));

	sei();

	/*	Start executing actions
	 *
	 */
	initActions();											// initialize various pointers

	timerEnable = TRUE;

	for (;;) {
		while (usart0inBufferCount() > 0)					// continuously check for a command from the client ..
			parse();										// .. and parse the input if any was received
		if (timerWakeup == TRUE && timerEnable == TRUE) {	// wakeup is set to TRUE every minute via timer1 interrupt routine
			checkActions(NORMAL);							// execute any actions
			timerWakeup = FALSE;							// wait for timer1 interrupt routine to clear wakeup again in a minute
		}
	}
	return 0;
}


/*	Client request parser
 *
 */
void parse(void)
{
	while (usart0inBufferCount() > 0) {
		switch (getch()) {
			case 'A':	timerEnable = TRUE;			// start executing timer actions every minute
						putch(reqOK);
						break;
			case 'B': 	timerEnable = FALSE;		// stop the timer from executing actions
						disableTimeOut = 10;		// the timer will automatically be re-enabled after 10 minutes
						verbose = FALSE;			// no more need for verbose once the timer has stopped
						putch(reqOK);
						break;
			case 'C': 	getTime();					// send the DS1307 date and time to the client
						break;
			case 'D':	if (setTime() == ERROR)		// receive date and time from the client and set the DS1307
							putch(reqERROR);
						else
							putch(reqOK);
						break;
			case 'E':	getAction();				// send a specific action to the client
						break;
			case 'F':	if (setAction() == ERROR)	// receive a specific action from the client and write to memory
							putch(reqERROR);
						else
							putch(reqOK);
						break;
			case 'G': 	if (switchUnit() == ERROR)	// receive and action from the client which must be executed immediately
							putch(reqERROR);
						else
							putch(reqOK);
						break;
			case 'H':	getInfo();					// send device info to the client
						break;
			case 'I':	setInfo();					// receive device info from the client
						break;
			case 'J':	verbose = TRUE;				// enable debugging output (how? connect via terminal, send 'A' and then 'I')
						putch(reqOK);
						printf("\r\nVerbose On\r\n");
						break;
			case 'K':	// do not use watchdog reset on a mySmartControl; if you do the device will not exit from the bootloader
						//wdt_enable(WDTO_15MS);	// set watchdog timer to 15mS
						//WDTCSR = 0x08;
						for (;;) { }				// wait for user to press reset
						break;
			default:	putch(reqERROR); 			// unknown request, ignore
						break;
		} // switch
	} // while
}


/*	Send current DS1307 date and time to the client.
 *
 *	Message sent (7 bytes):
 *
 *	0	day of date (as integer)
 *	1	month of date (as integer)
 *	2	year of date (as integer)
 *	3	weekday of date (as integer)
 *	4	hours of time (as integer)
 *	5	minutes of time (as integer)
 *	6	seconds of time (as integer)

 *	Return: OK when successful, ERROR if date and time could not be retrieved
 *
 */
int getTime()
{
	datetime dt;

	if (DS1307GetTime(&dt) == ERROR)
		return ERROR;

	putch(dt.dd);
	putch(dt.mm);
	putch(dt.yy);
	putch(dt.day);
	putch(dt.hrs);
	putch(dt.min);
	putch(dt.sec);

	return OK;
}


/*	Receive new date and time from client and set the DS1307 internal clock.
 *	Checks for validity of date and time
 *
 *	Message received (7 bytes):
 *
 *	0	day of date (as integer)
 *	1	month of date (as integer)
 *	2	year of date (as integer)
 *	3	weekday of date (as integer)
 *	4	hours of time (as integer)
 *	5	minutes of time (as integer)
 *	6	seconds of time (as integer)
 *
 *	Return: OK when successful, ERROR if invalid date or time or when DS1307 could not be written
 *
 */
int setTime()
{
	datetime dt;

	dt.dd  = getch();
	dt.mm  = getch();
	dt.yy  = getch();
	dt.day = getch();
	dt.hrs = getch();
	dt.min = getch();
	dt.sec = getch();

	if (dt.dd < 1 || dt.dd > 31)
		return ERROR;
	if (dt.mm < 1 || dt.mm > 12)
		return ERROR;
	if (dt.yy > 99)
		return ERROR;
	if (dt.day < 1 || dt.day > 7)
		return ERROR;
	if (dt.hrs > 23)
		return ERROR;
	if (dt.min > 59)
		return ERROR;
	if (dt.sec > 59)
		return ERROR;

	return DS1307SetTime(&dt);
}


/*	Retrieve an action from memory and send it to client.
 *
 *	Client first sends a 16-bit unsigned integer containing the index of the action to retrieve.
 *	Server then sends the action content to the client.
 *
 *	Message received (2 bytes):
 *
 *	0		low byte of index (as integer)
 *	1		high byte of index (as integer)
 *
 *	Message sent (SIZEOF(ACTION) bytes):
 *
 *	0-9		content of action
 *
 *	Return: OK when successful, ERROR in case of error reading the action from memory.
 *
 */
int getAction()
{
	uint16_t index;
	uint8_t i, lo, hi, data[sizeof(ACTION)];

	lo = getch();
	hi = getch();

	index = (hi * 0xFF) + lo;

	if (readAction(index, (ACTION *)&data) == ERROR)
		return ERROR;

	for (i = 0; i < sizeof(ACTION); i++)
		putch(data[i]);

	return OK;
}


/*	Receive an action from the client and store it in memory.
 *
 *	Client first sends a 16-but unsigned integer containing the index of the action,
  * followed by the content of the action.
 *
 *	Message received (2 + SIZEOF(ACTION) bytes):
 *
 *	0		low byte of index (as integer)
 *	1		high byte of index (as integer)
 *	2-12	content of action
 *
 */
int setAction()
{
	uint16_t index;
	uint8_t i, lo, hi, data[sizeof(ACTION)];

	lo = getch();
	hi = getch();

	for (i = 0; i < sizeof(ACTION); i++)
		data[i] = getch();

	index = (hi * 0xFF) + lo;
	
	if (writeAction(index, (ACTION *)&data) == ERROR)
			return ERROR;

	return OK;
}


/*	Receive a command from the client to immediately send to a switch.
 *
 *	Message received (3 bytes):
 *
 *	0		major device id (as character, values: 'A' to 'P')
 * 	1		minor device id (as integer, values: 1 to 16)
 * 	2		command (as integer, values: 0 or 1 (off / on))
 *
 *	Return: OK if parameter values were valid, else ERROR
 *
 */
int switchUnit()
{
	uint8_t major, minor, command;

	major   = getch();
	minor   = getch();
	command = getch();

	if ((char)major >= 'A' && (char)major <= 'P')
		if (minor >= 1 && minor <= 16)
			if (command >= 0 && command <= 1) {
				sendSignal(major, minor, command);
				return OK;
			}
	return ERROR;
}


/*	Transmit a string with 24 bytes of device info to the client.
 *
 *	Message sent (24 bytes):
 *
 *	0-2		hardware version (as string)
 *	3-5		software version (as string)
 *	6-10	data-memory size (as string)
 *	11-15	number of valid actions in memory (as string)
 *	16-23	reserved for future use
 *
 */
int getInfo()
{
	int	i;
	char buffer[8];

	sprintf(&buffer[0], "%3d", hardware.version);		// hardware version as string[3]

	for (i = 0; i < 3; i++)
		putch(buffer[i]);

	sprintf(&buffer[0], "%3d", SOFTWAREVERSION);		// software version as string[3]

	for (i = 0; i < 3; i++)
		putch(buffer[i]);

	sprintf(&buffer[0], "%05lu", hardware.memorySize);	// data-memory size as string[5]

	for (i = 0; i < 5; i++)
		putch(buffer[i]);

	sprintf(&buffer[0], "%05d", countActions());		// number of valid actions in memory as string[5]

	for (i = 0; i < 5; i++)
		putch(buffer[i]);

	for (i = 0; i < 8; i++)								// 8 bytes reserved for future use as string[8]
		putch('0');

	return OK;
}


/*	Write system information to the DS1307 memory.
 *
 *	Used only by manufacturer to initialize the device.
 *
 *	Message received (8 bytes):
 *
 *	0		hardware version (as integer, values: 0 to 255)
 *	1		memory type (as integer, values: 0 for internal EEPROM, 1 for I2C EEPROM)
 *	2-5		memory size in bytes (as integer, little-endian (low byte first then high byte, is the AVR GCC standard))
 *	6		magic number = 0xFE
 *	7		magic number = 0xAB
 *
 */
int	setInfo(void)
{
	uint8_t	i, data[8];

	for (i = 0; i < 8; i++)
		data[i] = getch();

	if (data[6] == 0xFE && data[7] == 0xAB) {				// check magic number as security measure to avoid unintended writes
		DS1307WriteData(0x08, 6, (uint8_t *)&data[0]);

		cli();
		// wdt_enable(WDTO_15MS); 							// set watchdog timer to 15mS -> do not use on a mySmartControl
		for (;;) {}											// basic information has changed, wait for user to press reset
	}
	return ERROR;
}


/*	Initialize action counter to position based on the current time.
 *
 */
void initActions(void)
{
	checkActions(INIT);
}


/*	Check if time has passed between the previous and the current call of this function,
 *	and if so then execute any action in between.
 *
 *	type = INIT to increase the counter to the current time without executing actions
 *
 */
void checkActions(run_t type)
{
	static int prev;
	int	curr;
	datetime dt;

	DS1307GetTime(&dt);

	if (verbose == TRUE) {
		if (type == INIT)
			printf("start initializing actions: checkActions\r");
		printf("start checking actions on %02d-%02d-%02d (%d) %02d:%02d:%02d\r\n", dt.dd, dt.mm, dt.yy, dt.day, dt.hrs, dt.min, dt.sec);
	}

	curr = dt.hrs * 60 + dt.min;

	if (verbose == TRUE)
		printf("current minute=%d\r\n", curr);

	if (curr == prev) 										// no time passed between now and previous call
		return;

	if (curr > prev)										// new call done later then previous call
		executeActions(prev, curr, dt.dd, dt.mm, dt.yy, dt.day, type);
	else {													// new call earlier then previous call, moved past midnight
		executeActions(prev, 24*60, dt.dd, dt.mm, dt.yy, dt.day, type);
		executeActions(0, curr, dt.dd, dt.mm, dt.yy, dt.day, type);
	}
	prev = curr;

	if (verbose == TRUE) {
		if (type == INIT)
			printf("end initializing actions\r");
		printf("end checking actions\r\n");
	}
}


/*	Execute actions inside time interval
 *
 *	Time interval (from, to) expressed as minutes since 00:00
 *
 *	from = inclusive
 *	to 	 = exclusive
 *
 *	dd-mm-yy = date
 *	weekday  = weekday
 *	type	 = INIT to increase counter without executing actions
 *
 */
void executeActions(int from, int to, int dd, int mm, int yy, int weekday, run_t type)
{
	ACTION a;
	int	time;
	int	first;
	static int next;

	if (verbose == TRUE)
		printf("execute actions between %d (incl) and %d (excl) on date %d-%d-%d, day %d\r\n", from, to, dd, mm, yy, weekday);

	first = next;

	do {
		if (readAction(next, &a) == ERROR)					// Load the next action from EEPROM.
			break;

		if (a.valid == 0) {									// An invalid entry means we are at the end of the list.
			if (next == 0)									// If action[0] is invalid ...
				break;										// ... then the list is empty: stop
			next = 0;										// else we need to go back to the top ...
			continue;										// ... so wrap around and continue.
		}

		time = a.hrs * 60 + a.min;							// Minute at which action should run

		if (verbose == TRUE)
			printf("next action due at %d\r\n", time);

		if (time >= from && time < to) {					// Is the time the action should run inside the interval?
			if (a.day == 0 || a.day == weekday) {			// Are we on the right weekday (if weekday is relevant)?
				if (a.dd == 0 || a.mm == 0 || (a.dd == dd && a.mm == mm && a.yy == yy)) {	// Are we on the right date (if date is relevant)?
					if (verbose == TRUE)
						printf("action: %02d-%02d-%02d (%d) %02d:%02d %c-%02d=%d\r\n",
								a.dd, a.mm, a.yy, a.day, a.hrs, a.min, a.major, a.minor, a.cmd);
					if (type != INIT)						// INIT just advances counters and does not execute
						sendSignal(a.major, a.minor, a.cmd);// Execute the action
				}
			}
			next += 1;										// Goto the next action to execute.
			if (next == maxActionIndex)						// If we are at the end of the list ...
				next = 0;									// ... then wrap around to the top.
		} else
			break;
	} while (next != first); 								// Avoid looping (in case of list with single entry)
}


/*	Count the number of valid actions in the EEPROM.
 *
 *	Return: count, or 0 in case of EEPROM read-error
 *
 */
int countActions()
{
	ACTION action;
	int	count = 0;

	for (count = 0; count < maxActionIndex; count++) {
		if (readAction(count, &action) == ERROR) {
			count = 0;
			break;
		}
		if (action.valid == 0)
			break;
	}
	return count;
}


/*	Read the action at position index from the EEPROM.
 *
 *	Return: OK or ERROR (in case if invalid index or EEPROM read-error)
 *
 */
int	readAction(int index, ACTION *action)
{
	if (index < maxActionIndex)
		if (ee24C65ReadData(index * sizeof(ACTION), sizeof(ACTION), (uint8_t *)action) == sizeof(ACTION))
			return OK;

	return ERROR;
}


/*	Write action to the EEPROM at position index.
 *
 *	Return: OK or ERROR (in case of invalid index or EEPROM write-error)
 *
 */
int	writeAction(int index, ACTION *action)
{
	if (index < maxActionIndex) {
		if (action->valid == 1) {
			if (ee24C65WriteData(index * sizeof(ACTION), sizeof(ACTION), (uint8_t *)action) == sizeof(ACTION))
				return OK;
		} else {
			/* for an invalid action record only write field action->valid to the EEPROM */
			if (ee24C65WriteData(index * sizeof(ACTION), sizeof(uint8_t), (uint8_t *)action) == sizeof(uint8_t))
				return OK;
		}
	}
	return ERROR;
}
