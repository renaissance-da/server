/*
 * CharacterServer.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "CharacterServer.h"
#include "CharacterSession.h"

CharacterServer::CharacterServer(std::vector<std::pair<int, int> > &banlist,
				 int port) :
    BasicServer(port, "CharacterServer", banlist)
{
    // Nothing extra to do at the moment for character servers

}

CharacterServer::~CharacterServer()
{
}

BasicSession *CharacterServer::startSession(int sockfd, sockaddr_in *addr)
{
    return new CharacterSession(sockfd, addr->sin_addr.s_addr, time(NULL));
}

