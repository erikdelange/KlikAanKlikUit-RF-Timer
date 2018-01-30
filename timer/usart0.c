/*	usart0.h
 *
 *	Interrupt-driven serial I/O.
 *
 *	2009	E. de Lange
 */
#include "define.h"
#include "usart0.h"


/*	Transmit data structures - circular buffer
 *
 */
static uint8_t TxBuf[TxBufLength];
volatile static uint8_t TxWR, TxRD, numTx;

/*	Receive data structures - circular buffer
 *
 */
static uint8_t RxBuf[RxBufLength];
volatile static uint8_t RxWR, RxRD, numRx;


void usart0Init()
{
	UBRR0H = (uint8_t)((F_CPU/(BAUD*16L))-1)>>8;		// set baud rate
	UBRR0L = (uint8_t) (F_CPU/(BAUD*16L))-1;
   	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);					// 8N1
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0); 	// enable receive and transmit interrupts
   
	TxWR = 0; TxRD = 0; numTx = 0;	 					// initialize Tx and Rx buffer pointers
	RxWR = 0; RxRD = 0; numRx = 0;
}


/*	Interrupt routine to store a received byte in the circular input buffer.
 *	Overwrites the oldest byte if the buffer is full.
 *
 */
ISR(USART_RX_vect)
{
	RxWR++;	
	RxWR &= RxBufMask;									// efficient way to implement pointer wrapping
	RxBuf[RxWR] = UDR0;
	numRx++;
}


/*	Retrieve the next received byte, wait for a byte to become available.
 *
 */
uint8_t usart0ReadByte(void)
{
	while (numRx == 0);

	RxRD++;
	RxRD &= RxBufMask;
	numRx--; 

	return RxBuf[RxRD];
}


/*	Interrupt routine to transmit bytes from the buffer.
 *
 */
ISR(USART_UDRE_vect)
{
	if (numTx > 0) {
		TxRD++;
		TxRD &= TxBufMask;
		UDR0 = TxBuf[TxRD]; 
		numTx--; 
	} else
		UCSR0B &= ~(1<<UDRIE0);	 						// no more bytes, disable transmitter
}


/*	Write a byte to the transmission buffer, wait for space in buffer.
 *
 */
void usart0WriteByte(uint8_t data)
{
	while (numTx == TxBufMask);

	TxWR++;
	TxWR &= TxBufMask;
	TxBuf[TxWR] = data;
	numTx++;
	UCSR0B |= 1<<UDRIE0;								// enable transmitter (UDRE interrupt)
}


uint8_t usart0inBufferCount(void)
{
	return numRx;
}
