/*
 * DataStream.h
 *
 * An interface to anything with appending functionality
 *
 *  Created on: 2012-12-07
 *      Author: per
 */

#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <string>

class IDataStream
{

public:
    virtual ~IDataStream()
    {
    }
    ;

    virtual void appendByte(char) = 0;
    virtual void appendBytes(unsigned int, char const *) = 0;
    virtual void appendString(unsigned char, char const *) = 0;
    virtual void appendInt(int) = 0;
    virtual void appendShort(short) = 0;

    virtual char extractByte() = 0;
    virtual std::string extractString() = 0;
    virtual char *getCurPtr() = 0;
    virtual int extractInt() = 0;
    virtual short extractShort() = 0;

    virtual void skip(int) = 0;

};

#endif /* DATASTREAM_H_ */
