/*
 * LoginServer.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef LOGINSERVER_H_
#define LOGINSERVER_H_

#include "BasicServer.h"
#include <stdlib.h>

class LoginServer: public BasicServer
{
public:
    LoginServer(std::vector<std::pair<int, int> > &banlist, int port = 2610);
    virtual ~LoginServer();

    BasicSession *startSession(int sockfd, sockaddr_in *addr);
};

#endif /* LOGINSERVER_H_ */
