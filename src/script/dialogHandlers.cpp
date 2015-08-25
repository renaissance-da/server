/*
 * crc.cpp
 *
 *  Created on: 2012-12-16
 *      Author: per
 */



void dialogDecrypt(unsigned char key0, unsigned char key1, unsigned short msgLen, char *msg)
{
	key1 ^= (key0 - 0x2d);
	unsigned char dataKey = key1 + 0x2A;

	for (unsigned short i = 0; i < msgLen; i++) {
		msg[i] ^= dataKey++;
	}
}
