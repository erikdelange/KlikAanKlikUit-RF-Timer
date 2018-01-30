/*	i2c.c
 *
 *	Routines sending and receiving data via I2C.
 *
 *	Source: twitest.c from avr-libc examples
 *
 *	Website:http://www.nongnu.org/avr-libc/user-manual/group__twi__demo.html
 *
 *	2008
 */
#include "define.h"
#include "i2c.h"


inline void i2cSendStart(void)				// send start condition
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
}

inline void i2cSendAck(void)				// send acknowledge
{
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
}

inline void i2cSendNack(void)				// send not-acknowledge
{
	TWCR = (1<<TWINT)|(1<<TWEN);
}

inline void i2cSendStop(void)				// send stop condition
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}
 
inline void i2cWaitForComplete(void)		// wait for transmission
{
	while (!(TWCR & (1<<TWINT)));
}

inline void i2cWaitForStop(void)			// wait for stop
{
	while (!(TWCR & (1<<TWSTO)));
}

inline void i2cSendByte(uint8_t data)
{
	TWDR = data;							// save data to the TWDR
	TWCR = (1<<TWINT)|(1<<TWEN);			// begin send
}


/* Maximum number of iterations to wait for a device to respond after a
 * selection. Should be large enough to allow for a pending write to
 * complete, but low enough to properly abort an infinite loop in case
 * a slave is broken or not present at all. With 100 kHz TWI clock,
 * transferring the start condition and SLA+R/W packet takes about 10
 * µs. The longest write period is not supposed to exceed ~ 10 ms.
 * Thus normal operation should not require more than 100 iterations
 * to get the device to respond to a selection.
 *
 */
#define MAX_ITER	200


void i2cInit(void)
{
	TWSR = 0;								// set prescaler to 1
	TWBR = (F_CPU / 100000UL - 16) / 2;		// 100kHz mode
}


/* Read "len" bytes from I2C device starting at "addr" into "d".
 *
 * This requires two bus cycles: during the first cycle the device
 * will be selected (master transmitter mode) and the address
 * transfered.
 *
 * The second bus cycle will reselect the device (repeated start
 * condition, going into master receiver mode) and transfer the data
 * from the device to the TWI master. Multiple bytes can be
 * transfered by ACKing the client's transfer. The last transfer will
 * be NACKed, which the client will interpret as indication to not
 * initiate further transfers.
 *
 * Uses 1 or 2 bytes addresses ("addrbytes" = 1 or 2). Devices with 256
 * bytes or less memory locations (DS1307, 24C02) use 1 bytes addresses,
 * larger ones 2 byte.
 *
 * Returns -1 in case of an error, or else the number of bytes read
 * which will equal "len".
 * 
 */
int i2cReadData(uint8_t dev, uint8_t addrbytes, uint16_t addr, int len, uint8_t *d)
{
	uint8_t n = 0;
	int r = 0;

	// First cycle: master transmitter mode

restart:
	if (n++ >= MAX_ITER)
		return (-1);

begin:
	i2cSendStart();							// send start condition
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_REP_START:					// OK, but should not happen
		case TW_START:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
 			return (-1);					// error: not in start condition
											// NB: do /not/ send stop condition
    }

	i2cSendByte(dev & 0xFE);				// send slave address + write
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_MT_SLA_ACK:
			break;
		case TW_MT_SLA_NACK:				// nack during select: device busy writing
			goto restart;
		case TW_MT_ARB_LOST:				// re-arbitrate
			goto begin;
		default:
			goto error;						// must send stop condition
    }

	if (addrbytes == 2)	{					// 1 or 2 bytes for address?
		i2cSendByte(addr>>8);				// set register
		i2cWaitForComplete();				// wait for transmission

		switch (TW_STATUS) {
			case TW_MT_DATA_ACK:
				break;
			case TW_MT_DATA_NACK:
				goto quit;
			case TW_MT_ARB_LOST:
				goto begin;
			default:
				goto error;					// must send stop condition
    	}
	}

	i2cSendByte(addr & 0xFF);
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			goto quit;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;						// must send stop condition
	}


	// Next cycle(s): master receiver mode
  
   	i2cSendStart();							// send (rep.) start condition
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_START:						// OK, but should not happen
		case TW_REP_START:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
 			return (-1);					// error: not in start condition
											// NB: do /not/ send stop condition
    }

	i2cSendByte(dev | TW_READ);				// send slave address + read
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_MR_SLA_ACK:
			break;
		case TW_MR_SLA_NACK:				// nack during select: device busy writing
			goto quit;
		case TW_MR_ARB_LOST:				// re-arbitrate
			goto begin;
		default:
			goto error;						// must send stop condition
    }

	for (; len > 0; len--) {
		if (len == 1)
			i2cSendNack();
		else
			i2cSendAck();
		i2cWaitForComplete();

		switch (TW_STATUS) {
			case TW_MR_DATA_NACK:
				len = 0;					// force end of loop
				// FALLTHROUGH
			case TW_MR_DATA_ACK:
				*d++ = TWDR;
				r++;
				break;
			default:
				goto error;
		}
	}

quit:
	i2cSendStop();							// send stop condition

	return (r);

error:
	r = -1;
	goto quit;
}


/* Write "len" bytes into I2C device starting at "addr" from "d".
 *
 * This is a bit simpler than the previous function since both the
 * address and the data bytes will be transfered in master transmitter
 * mode thus no re-selection of the device is necessary.
 *
 * Uses 1 or 2 bytes addresses ("addrbytes" = 1 or 2). Devices with 256
 * bytes or less memory locations (DS1307, 24C02) use 1 bytes addresses,
 * larger ones 2 byte.
 *
 * The function simply returns after writing, returning the
 * actual number of data byte written.
 *
 */
int i2cWriteData(uint8_t dev, uint8_t addrbytes, uint16_t addr, int len, uint8_t *d)
{
	uint8_t n = 0;
	int r = 0;

restart:
	if (n++ >= MAX_ITER)
    	return(-1);
  
begin:
	i2cSendStart();							// send start condition
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_REP_START:					// OK, but should not happen
		case TW_START:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
 			return (-1);					// error: not in start condition
											// NB: do /not/ send stop condition
    }

	i2cSendByte(dev & 0xFE);				// send slave address + write
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_MT_SLA_ACK:
			break;
		case TW_MT_SLA_NACK:				// nack during select: device busy writing
			goto restart;
		case TW_MT_ARB_LOST:				// re-arbitrate
			goto begin;
		default:
			goto error;						// must send stop condition
    }


	if (addrbytes == 2)	{					// 1 or 2 bytes for address?
		i2cSendByte(addr>>8);				// set register
		i2cWaitForComplete();				// wait for transmission

		switch (TW_STATUS) {
			case TW_MT_DATA_ACK:
				break;
			case TW_MT_DATA_NACK:
				goto quit;
			case TW_MT_ARB_LOST:
				goto begin;
			default:
				goto error;					// must send stop condition
	    }
	}

	i2cSendByte(addr);
	i2cWaitForComplete();					// wait for transmission

	switch (TW_STATUS) {
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			goto quit;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;						// must send stop condition
    }

	for (; len > 0; len--) {
		i2cSendByte(*d++);
		i2cWaitForComplete();				// wait for transmission

		switch (TW_STATUS) {
			case TW_MT_DATA_NACK:
				goto error;
			case TW_MT_DATA_ACK:
				r++;
	  			break;
			default:
				goto error;
		}
    }

quit:
	i2cSendStop();							// send stop condition

	return (r);

error:
	r = -1;
	goto quit;
}
