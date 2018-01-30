/*	i2c.h
 *
 *	Defines for I2C communication
 *
 *	2008	K.W.E. de Lange
 */
#ifndef _I2C_
#define _I2C_

#include <avr/io.h>
#include <util/twi.h>

void i2cInit(void);

int i2cReadData(uint8_t dev, uint8_t addrbytes, uint16_t addr, int len, uint8_t *d);

int i2cWriteData(uint8_t dev, uint8_t addrbytes, uint16_t addr, int len, uint8_t *d);

#endif /* _I2C_ */
