/*
 * DAPacket.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "DAPacket.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string.h>
#include "defines.h"
#include <errno.h>
#include <assert.h>
#include "network.h"
/*
 * Creates a new packet by reading off of the file descriptor given
 * Can throw a PacketError if the file descriptor couldn't be read from or if the packet is in an
 * invalid format
 */
DAPacket::DAPacket(int fd) :
    kRoot(0), ordinal(0), wasCyphered(false)
{
    code = 0;
    unsigned char testByte;
    unsigned short dataLen = 0;
    int nBytesReady;
    data = 0;

    if (Socket::getLen(fd, &nBytesReady) < 0) {
        throw UNKNOWN_SIZE;
    }

    if (nBytesReady == 0) {
        throw CONNECTION_CLOSED;
    }

    if (nBytesReady < 4) {
        throw SHORT_HEADER;
    }

    Socket::s_read(fd, (char *) &testByte, 1);
    Socket::s_read(fd, (char *) &dataLen, 2);
    dataLen = ntohs(dataLen);

    Socket::s_read(fd, (char *) &code, 1);

    //in order, bad header, fake dataLen, too much data, and no key root given
    if (testByte != 0xaa || dataLen > (nBytesReady - 3)
        || dataLen > MAX_PACKET_LEN || dataLen < 4) { // Dont try to read too much/little
        throw BAD_HEADER;
    }

    //Raw is the array which can be used to send or receive. It will always reserve 4 bytes at the beginning for the header
    pData = data = raw + 4; //increment when decrypting to skip the ordinal

    //The problem is that data which shouldn't be decrypted should have dataLength = dataLen-1
    //When we want to cypher it we could shrink it by 3, requires appending the key, but i guess its ok
    dataLength = dataLen - 1; //the code doesn't count internally
    Socket::s_read(fd, data, dataLength);
}

DAPacket::DAPacket(int code, char const *data, unsigned short dataLen) :
    dataLength(dataLen), code(code), kRoot(0), ordinal(0), wasCyphered(false)
{
    pData = this->data = raw + 4;
    memcpy(this->data, data, dataLength);
}

DAPacket::DAPacket(DAPacket *p) :
    kRoot(0), ordinal(0)
{
    data = raw + (p->data - p->raw);
    pData = raw + (p->pData - p->raw);
    dataLength = p->dataLength;
    code = p->code;

    //For now, assume uncyphered at start, probably not useful to do with cyphered packet
    assert(p->wasCyphered == false);

    wasCyphered = p->wasCyphered; //TODO consider what to do with ord
    memcpy(data, p->data, dataLength);
}

DAPacket::~DAPacket()
{

}

unsigned short DAPacket::getCode()
{
    return code;
}

char *DAPacket::getKeyRoot()
{
    return kRoot;
}

char DAPacket::getOrd()
{
    assert(wasCyphered);
    return ordinal;
}

void DAPacket::writeData(int fd)
{
    assert(dataLength + 11 < MAX_PACKET_LEN);

    raw[0] = 0xaa;
    raw[1] = (dataLength + (wasCyphered ? 5 : 1)) / 0x100; //+1 for code, +4 for key root and ordinal
    raw[2] = (dataLength + (wasCyphered ? 5 : 1)) % 0x100;
    raw[3] = code;

    int flags =
#ifdef WIN32
        0;
#else
        MSG_NOSIGNAL;
#endif
    if (send(fd, raw, dataLength + (wasCyphered ? 8 : 4), flags) == -1) {
        if (errno == EINTR)
            fprintf(stderr,
                "An unignored interrupt prevented a send request!\n");
        else if (errno == EINVAL)
            fprintf(stderr, "Invalid arguments provided to a send request!\n");
        else if (errno == EMSGSIZE)
            fprintf(stderr,
                "Attempted to send a packet which was too long (%d) bytes\n",
                dataLength + (wasCyphered ? 8 : 4));
        else if (errno == ENOBUFS || errno == ENOMEM)
            fprintf(stderr,
                "A packet buffer is full, so a packet could not be sent\n");
    }
}

char *DAPacket::getDataPtr()
{
    return data;
}

unsigned short DAPacket::getDataLen()
{
    return dataLength;
}

void DAPacket::cyphered()
{
    if (!wasCyphered) {
        wasCyphered = true;
        pData = data = raw + 5;
        dataLength -= 4; //ordinal and last 3 bytes disappear
        ordinal = raw[4];
        kRoot = data + dataLength;
    }
}

//Appends the length, as a byte, and then len bytes from newData onto the packet
void DAPacket::appendString(unsigned char len, char const *newData)
{
    assert(len + dataLength + 5 <= MAX_PACKET_LEN);
    assert(!wasCyphered);

    data[dataLength] = len;
    memcpy(data + dataLength + 1, newData, len);
    dataLength += len + 1;

}

//Appends len bytes of newData onto the packet
void DAPacket::appendBytes(unsigned int len, char const *newData)
{
    assert(len + dataLength + 4 <= MAX_PACKET_LEN);
    assert(!wasCyphered);

    memcpy(data + dataLength, newData, len);
    dataLength += len;

}

void DAPacket::appendByte(char b)
{
    assert(dataLength + 4 < MAX_PACKET_LEN);
    assert(!wasCyphered);

    data[dataLength++] = b;

}

void DAPacket::appendInt(int x)
{
    assert(dataLength + 8 <= MAX_PACKET_LEN);
    assert(!wasCyphered);

    *((int *) (data + dataLength)) = htonl(x);
    dataLength += 4;

}

void DAPacket::appendShort(short x)
{
    assert(dataLength + 6 <= MAX_PACKET_LEN);
    assert(!wasCyphered);

    *((short *) (data + dataLength)) = htons(x);
    dataLength += 2;

}

std::string DAPacket::extractString()
{
    if (pData - data >= dataLength) {
        throw PAST_END;
    }
    unsigned char len = *(pData++);
    if ((pData + len) - data > dataLength) {
        throw PAST_END;
    }
    std::string ret(pData, len);
    pData += len;
    return ret;
}

char DAPacket::extractByte()
{
    if (pData - data >= dataLength) {
        throw PAST_END;
    }
    return *(pData++);
}

int DAPacket::extractInt()
{
    if (pData + 3 - data >= dataLength) {
        throw PAST_END;
    }
    int ret = ntohl(*((int*) pData));
    pData += 4;
    return ret;
}

short DAPacket::extractShort()
{
    if (pData + 1 - data >= dataLength) {
        throw PAST_END;
    }
    short ret = ntohs(*((short*) pData));
    pData += 2;
    return ret;
}

char *DAPacket::getCurPtr()
{
    return pData;
}

void DAPacket::skip(int x)
{
    pData += x;
}

void DAPacket::setKRoot(char hi, char med, char lo)
{
    kRoot[0] = hi;
    kRoot[1] = med;
    kRoot[2] = lo;
}
