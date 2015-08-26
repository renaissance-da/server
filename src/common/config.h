/*
 * config.h
 *
 *  Created in: 2012
 *      Author: per
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "libconfig.h++"

namespace config
{
extern char ipaddr[4];
extern unsigned short loginPort, gamePort;
extern const char *motd, *log_conf;
extern libconfig::Config *cfg;
extern bool itemsPerish, itemsDrop, goldDrops, loseExp, loseHp, useMinPortal,
    useMaxPortal;
extern int expMod, skillMod, apMod, dropMod;
extern const char *db_conn_params;

bool parse(const char *configFile);

}
#endif
