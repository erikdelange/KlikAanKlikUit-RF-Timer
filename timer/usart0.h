/*	usart0.h
 *
 *	Defines for handling serial communication using USART0
 *
 *	2008	K.W.E. de Lange
 */
#ifndef _USART0_
#define _USART0_

#include <avr/io.h>
#include <avr/interrupt.h>

#define TxBufLength 32
#define TxBufMask 	TxBufLength - 1
#define RxBufLength 128
#define RxBufMask 	RxBufLength - 1

/*	Pointer wrapping in the circular buffers is implemented by masking unused high bits.
 *	This only works if the mask can be a power of 2. 
 *
 */
#if (TxBufLength & TxBufMask)
#error TxBufLength size is not a power of 2
#endif

#if (RxBufLength & RxBufMask)
#error RxBufLength size is not a power of 2
#endif

void usart0Init(void);
void usart0WriteByte(uint8_t data);
uint8_t usart0ReadByte(void);
uint8_t usart0inBufferCount(void);

#define getch	usart0ReadByte
#define putch	usart0WriteByte

#endif /* _USART0_ */
