/*
 * network.h
 *
 *  Created in: 2012
 *      Author: per
 */
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <arpa/inet.h>
#endif

namespace Socket
{

#ifdef WIN32
int s_close(SOCKET s);
void flush(SOCKET s);
void s_read(SOCKET s, char *buffer, int len);
int getLen(SOCKET s, int *len);
#else
int s_close(int fd);
void flush(int fd);
void s_read(int fd, char *buffer, int len);
int getLen(int fd, int *len);
#endif
}
