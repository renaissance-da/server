/*
 * crc.h
 *
 *  Created on: 2012-12-12
 *      Author: per
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

unsigned short getChecksum(unsigned char *buf, int bufLen);
uint32_t getCrc32(unsigned char *buf, int bufLen);
int compress(unsigned char *bufIn, unsigned char *bufOut, int bufInLen,
	     int bufOutLen);

#endif /* CRC_H_ */
