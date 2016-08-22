/*
 * BasicServer.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "BasicServer.h"
#include <list>
#include <time.h>
#include <sstream>
#include "random.h"
#include "types.h"
#include "network.h"

#ifdef WIN32
#define close closesocket
#endif

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

// Create a server which will use a separate thread to listen on the port given
// When a connection is ready on the port given, the virtual method startSession will be invoked
BasicServer::BasicServer(int port, char const *name,
    std::vector<std::pair<int, int> > &banlist) :
    port(port), listenFd(-1), running(true), name(name)
{
    for (auto p : banlist) {
        blacklist[p.first] = p.second;
    }

	listener = std::thread(startService, this);
	running = listener.joinable();
}

void BasicServer::restart()
{
    stop();
    running = true;
	listener = std::thread(startService, this);
	running = listener.joinable();
}

BasicServer::~BasicServer()
{
    if (running)
        stop();

}

// Stop the server - Closes all connections and terminates the listening thread
void BasicServer::stop()
{
    if (running) {
        running = false;
    }
    if (listener.joinable())
		listener.join();
    while (!clientList.empty()) {
        Socket::s_close(clientList.front()->getFd());
        delete clientList.front();
        clientList.pop_front();
    }
}

void BasicServer::startService(BasicServer *server)
{
    using std::list;

    int maxFd, res, newFd;
    fd_set currentConnections, readyConnections;
    sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);

    FD_ZERO(&currentConnections);
    //Open port
    server->listenFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef WIN32
    if (server->listenFd == INVALID_SOCKET) {
#else
    if (server->listenFd == -1) {
#endif
        server->running = false;
		return;
    }
    if (bind(server->listenFd, (sockaddr *) &addr, sizeof(addr)) == -1) {
        server->running = false;
        Socket::s_close(server->listenFd);
		return;
    }
    if (listen(server->listenFd, 100) == -1) {
        server->running = false;
        Socket::s_close(server->listenFd);
		return;
    }

    maxFd = server->listenFd;
    FD_SET(server->listenFd, &currentConnections);

    while (server->running) {
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        readyConnections = currentConnections;
        res = select(maxFd + 1, &readyConnections, NULL, NULL, &tv);
        if (res < 0) {
            //TODO check if the server needs to be restarted
            continue;
        }
        // Check open connections
        time_t timestamp = time(NULL);
        for (list<BasicSession *>::iterator it = server->clientList.begin();
            it != server->clientList.end();) {
            if (FD_ISSET((*it)->getFd(), &readyConnections)) {
                (*it)->dataReady();
                (*it)->updateReceived(timestamp);
            }
            else
                (*it)->updateTime(timestamp);

            if (!(*it)->isOpen()) {
                FD_CLR((*it)->getFd(), &currentConnections);
                delete (*it);
                it = server->clientList.erase(it);
            }
            else {
                it++;
            }

        }
        // Check port
        if (FD_ISSET(server->listenFd, &readyConnections)) {
            socklen_t addrLen = sizeof(addr);
            newFd = accept(server->listenFd, (sockaddr *) &addr, &addrLen);
            // Check blacklist
            std::lock_guard<std::mutex> lg(server->bl_mutex);
            if (server->blacklist.count(addr.sin_addr.s_addr)
                && server->blacklist[addr.sin_addr.s_addr] > time(NULL)) {
                close(newFd);
            }
            else {
                FD_SET(newFd, &currentConnections);
                server->clientList.push_back(
                    server->startSession(newFd, &addr));
                maxFd = MAX(maxFd, newFd);
            }
        }
    }

	Socket::s_close(server->listenFd);
}

std::string BasicServer::statusText()
{
    std::stringstream ret;
    if (running)
        ret << name << " is running on port " << port << ".";
    else
        ret << name << " has halted.";
    return ret.str();
}

bool BasicServer::functional()
{
    // TODO Add meaningful return value
    return running;
}

//Safely removes the session given
//Overload to send messages or perform other maintenance when disconnecting
void BasicServer::disconnect(BasicSession *s)
{

}

/**
 * Blacklist the given ip address, preventing further connections from being
 * accepted from that ip address.
 * \param[in] ip_addr The address to blacklist.
 */
void BasicServer::addToBlacklist(uint32_t ip_addr, int exp)
{
    std::lock_guard<std::mutex> lg(bl_mutex);
    blacklist[ip_addr] = exp;
}
