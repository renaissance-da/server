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

#ifdef WIN32
#include <WinSock2.h>
#include "random.h"
#else
#include <netinet/in.h>
#endif
#include <pthread.h>
#include <mutex>

class BasicServer
{
public:
    BasicServer(drand48_data *rngData, int port, char const *name,
        std::vector<std::pair<int, int> > &banlist);
    virtual ~BasicServer();
    void stop();
    bool functional();

    void restart();

    std::string statusText();

    virtual BasicSession *startSession(int sockfd, sockaddr_in *addr) = 0;
    void disconnect(BasicSession *s);
    void addToBlacklist(in_addr_t ip_addr, int exp);

private:
    std::list<BasicSession *> clientList;
    int port, listenFd;
    pthread_t listener;
    bool running;
    static void *startService(void *srv);
    std::string name;
    std::unordered_map<in_addr_t, int> blacklist;
    std::mutex bl_mutex;

protected:
    drand48_data *rng;
};

#endif /* BASICSERVER_H_ */
