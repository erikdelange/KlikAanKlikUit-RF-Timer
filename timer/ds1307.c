/*	ds1307.c
 *
 *	Routines for accessing a DS1307 Real-Time Clock
 *
 *	2008	K.W.E. de Lange
 */
#include "ds1307.h"
#include "utility.h"
#include "define.h" 


/*	Write datetime from struct TimeKeeper to DS1307.
 *
 *	Return: OK when successful, else ERROR
 *
 */
int	DS1307SetTime(datetime *TimeKeeper)
{
	datetime BCD_TimeKeeper;

	BCD_TimeKeeper.sec = int2bcd(TimeKeeper->sec) & 0x7F;	// always enable the oscillator
	BCD_TimeKeeper.min = int2bcd(TimeKeeper->min);
	BCD_TimeKeeper.hrs = int2bcd(TimeKeeper->hrs) & 0x3F;	// ensure 24 hrs mode (instead of AM/PM)
	BCD_TimeKeeper.day = int2bcd(TimeKeeper->day);
	BCD_TimeKeeper.dd  = int2bcd(TimeKeeper->dd);
	BCD_TimeKeeper.mm  = int2bcd(TimeKeeper->mm);
	BCD_TimeKeeper.yy  = int2bcd(TimeKeeper->yy);

	if (DS1307WriteData(0x00, 7, (uint8_t *)&BCD_TimeKeeper) != sizeof(datetime))
		return ERROR;

	return OK;
}


/*	Read current datetime from DS1307 into struct TimeKeeper.
 *
 *	Return: OK when successful, else ERROR
 *
 */
int	DS1307GetTime(datetime *TimeKeeper)
{
	datetime BCD_TimeKeeper;

	if (DS1307ReadData(0x00, 7, (uint8_t *)&BCD_TimeKeeper) != sizeof(datetime))
		return ERROR;

	TimeKeeper->sec = bcd2int(BCD_TimeKeeper.sec);
	TimeKeeper->min = bcd2int(BCD_TimeKeeper.min);
	TimeKeeper->hrs = bcd2int(BCD_TimeKeeper.hrs);
	TimeKeeper->day = bcd2int(BCD_TimeKeeper.day);
	TimeKeeper->dd  = bcd2int(BCD_TimeKeeper.dd);
	TimeKeeper->mm  = bcd2int(BCD_TimeKeeper.mm);
	TimeKeeper->yy  = bcd2int(BCD_TimeKeeper.yy);

	return OK;
}
