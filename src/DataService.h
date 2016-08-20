/*
 * DataService.h
 *
 *  Created on: 2012-11-30
 *      Author: per
 */

#ifndef DATASERVICE_H_
#define DATASERVICE_H_

#include <string>
#include "CharacterList.h"
#include <map>
#include "Map.h"
#include "DAPacket.h"
#include "Guild.h"
#include <mutex>

//TODO this should go in a module for managing server instances
void addToBlacklists(uint32_t ip_addr, int exp);

class DataService {
public:
	DataService();
	virtual ~DataService();

	static DataService *getService();

	void addMap(Map *m, unsigned short id) { maps[id] = m; }

	bool makeCharacter(std::string name, char const *pw, uint32_t ipaddr);
	bool setAttributes(std::string name, unsigned char hair, unsigned char hairColor, char gender);
	bool changePassword(std::string name, char const *pwOld, char const *pwNew);
	int prepareLogin(std::string name, char const *pw, unsigned int ip);
	Character *getCharacter(std::string name, CharacterSession *who, unsigned int ip);
	std::vector<Character *> *getCountryList();
	Map *getMap(unsigned int n);
	void tick();
	void clearAll();

	void freeCharacter(Character *c);

	void lock();
	void unlock();

	void sendToOthers(DAPacket *p, CharacterSession *c);
	void sendToAll(DAPacket *p, Character::Settings accept);
	void sendWhisper(CharacterSession *s, std::string &recip, std::string &msg);
	void groupChat(CharacterSession *s, std::string &msg);
	void guildChat(CharacterSession *s, std::string &msg);
	void enterMap(Character *c);
	bool tryChangeMap(Character *c, unsigned short map, unsigned short x, unsigned short y, unsigned short rad);
	void leaveMap(Character *c);
	void addMob(unsigned short map, unsigned short x, unsigned short y, unsigned short apr);
	void groupInvite(Character *m, std::string nm);
	void groupAccept(Character *m, std::string gn);
	void leaveGroup(Character *c);
	void groupListMembers(IDataStream *dp, Character *c);
	void groupDistributeExp(unsigned int exp, unsigned short level, int mobId, Entity *e);
	void broadcast(const char *msg);
	void storeItem(int charId, int itemId, int qty, short mod);
	void updateStoredItem(int charId, int itemId, int qty, short mod);
	void updateItem(int charId, unsigned short slot, int itemId, short qty, int dur, short mod, short idd);
	void removeItem(int charId, unsigned short slot);
	void addQuest(int charId, int questId, int qp, int timer);
	void updateQuest(int charId, int questId, int qp, int timer);
	void addLegend(int charId, int legendId, const char *textParam, int intParam, int timestamp);
	void updateLegend(int charId, int legendId, const char *textParam, int intParam, int timestamp);
	void deleteLegend(int charId, int legendId);
	Guild *createGuild(const char *name);
	bool addGuildMember(Guild *g, Character *c, Guild::Rank rank = Guild::MEMBER);
	bool removeGuildMember(Guild *g, const char *charName);
	bool promoteGuildMember(Guild *g, const char *charName, Guild::Rank newRank);
	void deleteGuild(Guild *g);
	void addSkill(int charId, int skillId, unsigned short slot);
	void removeSkill(int charId, unsigned short slot);
	void moveSkill(int charId, unsigned short oldSlot, unsigned short newSlot);
	void swapSkills(int charId, unsigned short slotOne, unsigned short slotTwo, int idOne, int idTwo);
	void addSecret(int charId, int skillId, unsigned short slot);
	void removeSecret(int charId, unsigned short slot);
	void moveSecret(int charId, unsigned short oldSlot, unsigned short newSlot);
	void swapSecrets(int charId, unsigned short slotOne, unsigned short slotTwo, int idOne, int idTwo);
	void updatePath(int charId, unsigned short path, unsigned short pathmask);
	void updateHairstyle(int charId, unsigned short style);
	void updateHaircolor(int charId, unsigned short color);
	void updateNation(int charId, unsigned short nation);
	void blacklist(const char *name, int priv);
	void banChar(const char *name, int priv);
	void recallChar(const char *name, int priv, Character *c);
	void gotoChar(const char *name, Character *c);

private:
	static DataService gleton;

	CharacterList *characters;
	std::mutex onlineLock, groupsLock;
	typedef std::lock_guard<std::mutex> guard_t;

	std::map<unsigned short, Map *> maps;
	unsigned char infoTick;
};

#endif /* DATASERVICE_H_ */
