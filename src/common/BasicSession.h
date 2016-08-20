/*
 * BasicSession.h
 *
 * Represents a session started by a basicserver
 * All sessions have a file descriptor associated with them, and an open/closed state
 *
 *  Created on: 2011-07-25
 *      Author: per
 */

#ifndef BASICSESSION_H_
#define BASICSESSION_H_
#include <stdlib.h>
#include <sstream>

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cstdint>

class BasicSession
{
public:
    BasicSession(int fd, int time, uint32_t clientIp);
    virtual ~BasicSession();

    virtual void dataReady() = 0;
    virtual void aboutToTimeout()
    {
    }
    void updateTime(int now);
    void updateReceived(int now)
    {
        lastRecv = now;
        this->now = now;
    }

    int getFd();
    bool isOpen();
    uint32_t getIp()
    {
        return clientIp;
    }
    std::string getIpString()
    {
	int ip_host = ntohl(clientIp);
	std::stringstream res;
	res << ((ip_host >> 24) & 0xFF) << "."
	    << ((ip_host >> 16) & 0xFF) << "."
	    << ((ip_host >> 8) & 0xFF) << "."
	    << (ip_host & 0xFF);
	return res.str();
    }

protected:
    int fd;
    bool open;
    uint32_t clientIp;
    int lastRecv, now;
};

#endif /* BASICSESSION_H_ */
