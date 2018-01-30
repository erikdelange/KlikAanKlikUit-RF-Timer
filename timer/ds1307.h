/*	ds1307.h
 *
 *	Defines for accessing a DS1307 Real-Time Clock.
 *
 *	2008	K.W.E. de Lange
 */
#ifndef _DS1307_
#define _DS1307_

#include "i2c.h"

#define DS1307	0xD0	/* DS1307 I2C address */

/*	The DS1307 is accessed via I2C, so the routines to read or write 
 *	are actually disguised I2C access routines.
 *
 */
#define DS1307ReadData(a, l, d)		i2cReadData(DS1307, 1, (uint16_t)a, l ,d)
#define DS1307WriteData(a, l, d)	i2cWriteData(DS1307, 1, (uint16_t)a, l, d)

#define	DS1307SetControl(c)			i2cWriteData(DS1307, 1, 0x0007, 1, (uint8_t*)c)
#define DS1307GetControl(c)			i2cReadData(DS1307, 1, 0x0007, 1, (uint8_t*)c)

/*	Structure matching the DS1307 RTC register layout
 *
 */
typedef struct DS1307TimeKeeper 
{
	uint8_t sec;
	uint8_t min;
	uint8_t hrs;
	uint8_t day;
	uint8_t dd;
	uint8_t mm;
	uint8_t yy;
} datetime;

int	DS1307SetTime(datetime *TimeKeeper);
int	DS1307GetTime(datetime *TimeKeeper);

#endif /* _DS1307_ */
