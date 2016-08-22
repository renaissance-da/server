/*
 * BasicServer.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef BASICSERVER_H_
#define BASICSERVER_H_
#include <sys/types.h>
#include "BasicSession.h"
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

#ifdef WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif
#include <mutex>

class BasicServer
{
public:
    BasicServer(int port, char const *name,
        std::vector<std::pair<int, int> > &banlist);
    virtual ~BasicServer();
    void stop();
    bool functional();

    void restart();

    std::string statusText();

    virtual BasicSession *startSession(int sockfd, sockaddr_in *addr) = 0;
    void disconnect(BasicSession *s);
    void addToBlacklist(uint32_t ip_addr, int exp);

private:
    std::list<BasicSession *> clientList;
    int port, listenFd;
    std::thread listener;
    bool running;
    static void startService(BasicServer *server);
    std::string name;
    std::unordered_map<uint32_t, int> blacklist;
    std::mutex bl_mutex;

};

#endif /* BASICSERVER_H_ */
