/*
 * BasicSession.cpp
 *
 *  Created on: 2011-07-25
 *      Author: per
 */

#include "BasicSession.h"
#include "network.h"

BasicSession::BasicSession(int fd, int time, uint32_t clientIp):
    fd(fd), open(true), clientIp(clientIp), lastRecv(time), now(time)
{

}

void BasicSession::updateTime(int now)
{
    if (this->now != now) {
        if (now - lastRecv > 10 && this->now - lastRecv <= 10)
            aboutToTimeout();
        this->now = now;
    }
}

BasicSession::~BasicSession()
{
    Socket::flush(fd);
    Socket::s_close(fd);
    //fsync(fd);
    //close(fd);
}

int BasicSession::getFd()
{
    return fd;
}

bool BasicSession::isOpen()
{
    return open && now - lastRecv < 15;
}

