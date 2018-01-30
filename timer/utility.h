/*	utility.h
 *
 *	Various utilities not related to a specific project
 *
 *	2008	K.W.E. de Lange
 */
#ifndef _UTILITY_
#define _UTILITY_

#define bitSet(p, m) ((p) |= (m))
#define bitClr(p, m) ((p) &= ~(m)) 

typedef enum {FALSE = 0, TRUE} boolean;

unsigned char int2bcd(unsigned char b);
unsigned char bcd2int(unsigned char b);

#endif /* _UTILITY_ */
