/*
 * LoginServer.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "LoginServer.h"
#include "LoginSession.h"
#include <list>
#include <sys/types.h>

#ifndef WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

LoginServer::LoginServer(std::vector<std::pair<int, int> > &banlist, int port):
    BasicServer(port, "LoginServer", banlist)
{
}

LoginServer::~LoginServer()
{
}

// Function to handle generating a new session using the file descriptor given
BasicSession *LoginServer::startSession(int sockfd, sockaddr_in *addr)
{
    return new LoginSession(sockfd, addr->sin_addr.s_addr, time(NULL));
}
