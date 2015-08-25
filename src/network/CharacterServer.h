/*
 * CharacterServer.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef CHARACTERSERVER_H_
#define CHARACTERSERVER_H_
#include "BasicServer.h"

class CharacterServer: public BasicServer
{
public:
    CharacterServer(drand48_data *rngData,
        std::vector<std::pair<int, int> > &banlist, int port = 2615);
    virtual ~CharacterServer();

    BasicSession *startSession(int sockfd, sockaddr_in *addr);
};

#endif /* CHARACTERSERVER_H_ */
