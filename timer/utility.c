/*	utility.h
 *
 *	Various utility routines
 *
 *	2008	K.W.E. de Lange
 */


/*	Convert a packed BCD byte to an 8-bit integer. 
 *
 *	In packed BCD every nibble of a byte represents a digit. 
 *
 *	The input (char b) is a byte containing big-endian packed BCD; so the high nibble (bits 4-7) 
 *	represents the 10's, the low nibble (bits 0-4) the 1's.
 *
 */
unsigned char bcd2int(unsigned char b)
{ 
	unsigned char r; 

	r  = ((b >> 4) & 0x0F) * 10; 
	r += (b & 0x0F); 

	return r; 
}


/*	Convert an 8-bit integer to a packed BCD byte.
 *
 *	In packed BCD every nibble of a byte represents a digit. 
 *
 *	The output (r) is a byte containing big-endian packed BCD; so the high nibble (bits 4-7) 
 *	represents the 10's, the low nibble (bits 0-4) the 1's.
 */
unsigned char int2bcd(unsigned char b)
{ 
	unsigned char r; 
	unsigned char tmp1, tmp2; 
    
	tmp1 = b / 10; 
	tmp2 = b - tmp1 * 10; 

	r  = (tmp1 << 4) & 0xF0; 
	r += (tmp2 & 0x0F); 

	return r;
}
