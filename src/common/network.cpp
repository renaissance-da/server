/*
 * network.cpp
 *
 *  Created in: 2012
 *      Author: per
 */
#include "network.h"

#ifdef WIN32
int Socket::s_close(SOCKET s) {return closesocket(s);}
void Socket::flush(SOCKET s) { /*TODO is there a way to sync sockets in windows?*/}
void Socket::s_read(SOCKET s, char *buffer, int len) {recv(s, buffer, len, 0);}
int Socket::getLen(SOCKET s, int *len) {return ioctlsocket(s, FIONREAD, (u_long*)len) == SOCKET_ERROR ? -1 : 0;}
#else
int Socket::s_close(int fd)
{
    return close(fd);
}
void Socket::flush(int fd)
{
    fsync(fd);
}
void Socket::s_read(int fd, char *buffer, int len)
{
    read(fd, buffer, len);
}
int Socket::getLen(int fd, int *len)
{
    return ioctl(fd, FIONREAD, len);
}
#endif
