/*
 * config.cpp
 *
 *  Created in: 2012
 *      Author: per
 */

#include "config.h"
#include <fstream>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

char config::ipaddr[sizeof(in_addr)];
unsigned short config::loginPort;
unsigned short config::gamePort;
const char *config::motd;
const char *config::db_conn_params;
libconfig::Config *config::cfg;
bool config::itemsPerish;
bool config::itemsDrop;
bool config::goldDrops;
bool config::loseExp;
bool config::loseHp;
bool config::useMinPortal;
bool config::useMaxPortal;
int config::expMod;
int config::apMod;
int config::skillMod;
int config::dropMod;

bool config::parse(const char *configFile)
{
    using namespace libconfig;

    cfg = new Config();
    const char *ipstring;

    try {
        cfg->readFile(configFile);
        ipstring = cfg->lookup("server_ip");
        loginPort = (unsigned int) cfg->lookup("login_port");
        gamePort = (unsigned int) cfg->lookup("game_port");
        motd = cfg->lookup("login_message");
        itemsPerish = cfg->lookup("perish");
        itemsDrop = cfg->lookup("drop_on_death");
        goldDrops = cfg->lookup("drop_gold_on_death");
        loseExp = cfg->lookup("lose_exp");
        loseHp = cfg->lookup("lose_hp");
        useMinPortal = cfg->lookup("use_min_portal_level");
        useMaxPortal = cfg->lookup("use_max_portal_level");
        expMod = cfg->lookup("exp_rate");
        apMod = cfg->lookup("ap_rate");
        skillMod = cfg->lookup("skill_rate");
        dropMod = cfg->lookup("drop_rate");
	db_conn_params = cfg->lookup("database_conn_params");
    }
    catch (FileIOException &fe) {
        fprintf(stderr,
            "Failed to read config file. (Have you copied daserver.conf.template into "
                "daserver.conf?\n");
        return false;
    }
    catch (SettingNotFoundException &se) {
        fprintf(stderr, "Failed to load setting from file: %s\n", se.getPath());
        return false;
    }
    catch (ConfigException &ce) {
        fprintf(stderr, "Failed to load settings from config file.\n");
        return false;
    }

    if (inet_pton(AF_INET, ipstring, (void *) ipaddr) != 1) {
        fprintf(stderr, "Couldn't parse ip from config file (given %s).\n",
            ipstring);
        return false;
    }
    return true;
}
