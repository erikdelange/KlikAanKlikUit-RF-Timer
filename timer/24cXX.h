/*	24cXX.h
 *
 *	Defines for accessing a 24cXX I2C EEPROM.
 *
 *	2009	K.W.E. de Lange
 */
#ifndef _24CXX_
#define _24CXX_

#include "i2c.h"

#define ee24C65	0xA0	/* 24C65 I2C address */

/*	The 24C65 a an 8Kb serial EEPROM. It is accessed via I2C, so the routines 
 *	to read- or write are actually disguised I2C access routines.
 *
 */
#define ee24C65ReadData(a, l, d)	i2cReadData(ee24C65, 2,(uint16_t)a, l, d)
#define	ee24C65WriteData(a, l, d)	i2cWriteData(ee24C65, 2,(uint16_t)a, l, d)

#endif /* _24C65_ */
