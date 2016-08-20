/*
 * DataLoaders.h
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef DATALOADERS_H_
#define DATALOADERS_H_

#include "Character.h"
#include "CharManager.h"
#include "log4cplus/logger.h"

//#define CONN_PARAMS "dbname=daserver_test"

namespace Database
{
    log4cplus::Logger log();
    
    int loadSkillDefs();
    int loadMaps(const char *mapDir);
    Character *loadCharacter(const char *charName);
    bool makeCharacter(const char *name, const char *pw);
    bool setAttributes(const char *name, unsigned char hair, unsigned char haircolor, char gender);
    bool changePassword(const char *name, const char *oldPw, const char *newPw);
    int testPassword(const char *name, const char *pw);
    void setPassword(const char *name, const char *pw);
    void getSkills(Character *c);
    void getBuffs(Character *c);
    void getItems(Character *c);
    void loadSpawners();
    void addLegendItem(int id, short icon, short color, const char *prefix, const char *text);
    void delLegend(int id, const char *prefix);
    void loadItems();
    void loadLegends();
    void loadMobs();
    void loadGuilds();
    std::vector<std::pair<int, int> > loadBanList();
    void banIp(int ipaddr, int expiry);

    int createGuild(const char *name);
    void addToGuild(int charId, int guildId, short rank);
    void removeFromGuild(const char *charName);
    void updateGuildRank(const char *charName, short rank);
    void deleteGuild(int guildId);

    bool initDb();
    void closeDb();

    extern CharManager *cm;
}

#endif /* DATALOADERS_H_ */
