/*	remote.h
 *
 *	Defines for controlling remote switches
 *
 *	2009	K.W.E. de Lange
 */
#ifndef _REMOTE_
#define _REMOTE_

void sendSignal(uint8_t major, uint8_t minor, uint8_t command);

#endif /* _REMOTE_ */
