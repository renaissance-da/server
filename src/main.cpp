/*
 * main.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "CharacterServer.h"
#include "LoginServer.h"
#include "DataService.h"
#include "GameEngine.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include "BaseItem.h"
#include <fcntl.h>
#include <signal.h>
#include "DataLoaders.h"
#include "npc_bindings.h"
#include "random_engines.h"
#include "config.h"
#include "log4cplus/configurator.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "LoginSession.h"

#ifdef WIN32
#include "log4cplus/initializer.h"
#define sleep(x) Sleep(1000*x)
#include <conio.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

CharacterServer *charServer;
LoginServer *loginServer;

#ifndef WIN32
void interrupted(int sig)
{
	switch (sig) {
	case SIGQUIT:
		charServer->stop();
		loginServer->stop();
		Database::closeDb();
		delete charServer;
		delete loginServer;
		exit(0);
	}
}
#endif

void addToBlacklists(uint32_t ip_addr, int exp)
{
	charServer->addToBlacklist(ip_addr, exp);
	loginServer->addToBlacklist(ip_addr, exp);
}

int main()
{
#ifdef WIN32
	log4cplus::Initializer initializer;
#endif
	
	using namespace Database;
	if (!config::parse("daserver.conf")) {
	    return 1;
	}

	std::ifstream lconf(config::log_conf);
	if (lconf.good()) {
	    log4cplus::PropertyConfigurator config(lconf);
	    config.configure();
	}
	else {
	    log4cplus::BasicConfigurator config;
	    config.configure();
	}

	log4cplus::Logger log = log4cplus::Logger::getInstance("renaissance");

	if (!initDb()) {
		LOG4CPLUS_FATAL(log, "Couldn't start a connection with the database. Quitting...");
		return 1;
	}
	loadItems();
	loadMaps("maps");
	loadSkillDefs();
	loadMobs();
	loadGuilds();
	loadSpawners();
	loadLegends();
	initScripts();
	LoginSession::initMessages();
	std::vector<std::pair<int, int> > banlist(loadBanList());

#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData)) {
		fprintf(stderr, "Failed to call WSAStartup\n");
		return 1;
	}
#else
	int flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, O_NONBLOCK | flags);
#endif

	charServer = new CharacterServer(banlist);
	loginServer = new LoginServer(banlist);
	GameEngine *ge = new GameEngine(DataService::getService());
	bool csUp = false, lsUp = false;

#ifndef WIN32
	struct sigaction s;
	s.sa_handler = interrupted;
	sigemptyset (&s.sa_mask);
	s.sa_flags = 0;

	//signal(SIGQUIT, interrupted) == SIG_ERR);
	int r = sigaction(SIGQUIT, &s, 0);
	if (r)
		LOG4CPLUS_ERROR(log, "Failed to set up QUIT action, force quitting the server may result in loss of data.");
#endif

	bool run = true;
	while (run)
	{
		if (!charServer->functional()) {
			LOG4CPLUS_WARN(log, charServer->statusText() << " Restarting in 1 second...");
			charServer->restart();
			csUp = false;
		}
		else if (csUp == false) {
		    csUp = true;
		    LOG4CPLUS_INFO(log, charServer->statusText());
		}
		if (!loginServer->functional()) {
			LOG4CPLUS_WARN(log, loginServer->statusText() << " Restarting in 1 second...");
			loginServer->restart();
			lsUp = false;
		}
		else if (lsUp == false) {
		    lsUp = true;
		    LOG4CPLUS_INFO(log, loginServer->statusText());
		}
		if (!ge->isRunning()) {
			LOG4CPLUS_FATAL(log, "Game engine has stopped running. Terminating...");
			exit(1);
		}
		sleep(1);
		char buffer[200];
#ifdef WIN32
		if (_kbhit())
			run = false;
#else
		int r = read(0, buffer, 199);
		if (r != -1 || errno != EAGAIN)
		{
			if (r)
				buffer[r-1] = '\0';
			else
				buffer[0] = '\0';

			if (strncmp(buffer, "broadcast", strlen("broadcast")) == 0) {
				DataService::getService()->broadcast(buffer + strlen("broadcast") + 1);
			}
			else if (strcmp(buffer, "quit") == 0)
				run = false;
		}
#endif
	}

	delete loginServer;
	delete charServer;
	delete ge;
	closeDb();

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}
