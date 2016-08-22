/*
 * crc.cpp
 *
 *  Created on: 2012-12-12
 *      Author: per
 */

#include "crc.h"
#include "zlib.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"

unsigned short table[256];

//credit to http://da-dev.wikispaces.com/Nexon+CRC16+Algorithm
bool initTable(unsigned short poly)
{
    for (int i = 0; i < 256; i++) {
        unsigned short ent = 0;
        unsigned short a = (unsigned short) (i << 8);
        for (int j = 0; j < 8; j++) {
            if ((ent ^ a) & 0x8000) {
                ent = (unsigned short) ((ent << 1) ^ poly);
            }
            else {
                ent <<= 1;
            }
            a <<= 1;
        }
        table[i] = ent;
    }

    return true;
}

bool b = initTable(0x1021);

unsigned short getChecksum(unsigned char *buf, int len)
{
    unsigned short r = 0;

    for (int i = 0; i < len; i++) {
        r = (unsigned short) ((r << 8) ^ (table[r >> 8]) ^ buf[i]);
    }
    return r;
}

uint32_t getCrc32(unsigned char *buf, int bufLen)
{
    uint32_t crcInit = crc32(0L, Z_NULL, 0);
    return crc32(crcInit, buf, bufLen);
}

int compress(unsigned char *bufIn, unsigned char *bufOut, int bufInLen,
	     int bufOutLen)
{
    z_stream z;
    log4cplus::Logger log = log4cplus::Logger::getInstance("common");

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
	LOG4CPLUS_ERROR(log, "Failed to initialize z_stream object.");
	return -1;
    }

    z.avail_in = bufInLen;
    z.next_in = bufIn;
    z.avail_out = bufOutLen;
    z.next_out = bufOut;
    if (deflate(&z, Z_FINISH) == Z_STREAM_ERROR) {
	LOG4CPLUS_ERROR(log, "Failed to compress data.");
	return -1;
    }

    int len = bufOutLen - z.avail_out;
    deflateEnd(&z);
    if (len > 0)
	return len;
    else
	return -1;
}
