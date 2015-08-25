/*
 * DataService.cpp
 *
 *  Created on: 2011-09-04
 *      Author: per
 */

#include "DataService.h"
#include "defines.h"
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "crc.h"
#include "Mob.h"
#include "srv_proto.h"
#include <algorithm>
#include "DataLoaders.h"
#include "CharManager.h"
#include "LockSet.h"

#ifdef WIN32
#define snprintf _snprintf
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "defines.h"
#include <assert.h>

DataService DataService::gleton;

DataService::DataService():
#ifndef NDEBUG
onlineLock(PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP)
#else
onlineLock(PTHREAD_MUTEX_INITIALIZER)
#endif
{
	characters = new CharacterList(std::string("characters/"));
	infoTick = 0;
}

DataService::~DataService() {
	delete characters;

	pthread_mutex_destroy(&onlineLock);
}

bool DataService::makeCharacter(std::string name, char const *pw, in_addr_t ipaddr)
{
	lock();

	bool ret = characters->makeCharacter(name, pw, ipaddr);
	unlock();
	return ret;
}

bool DataService::setAttributes(std::string name, unsigned char hair, unsigned char hairColor, char gender)
{
	lock();
	bool ret = characters->setAttributes(name, hair, hairColor, gender);
	unlock();
	return ret;
}

bool DataService::changePassword(std::string name, char const *pwOld, char const *pwNew)
{
	lock();
	bool ret = characters->changePassword(name, pwOld, pwNew);
	unlock();
	return ret;
}

DataService *DataService::getService()
{
	return &gleton;
}

void DataService::sendToAll(DAPacket *p, Character::Settings accept)
{
	lock();
	for (CharacterList::iterator it = characters->begin(); it != characters->end(); it++) {
		if (it->second->getCharacter()->getSettings() & accept) {
			DAPacket r(p);
			it->second->sendPacket(&r);
		}
	}
	unlock();
}

Map *DataService::getMap(unsigned int n)
{
	assert(maps.count(n));
	std::map<unsigned short, Map *>::iterator map = maps.find(n);
	if (map != maps.end())
		return map->second;
	return 0;
}

void DataService::sendToOthers(DAPacket *p, CharacterSession *s)
{
	lock();
	for (CharacterList::iterator it = characters->begin(); it != characters->end(); it++) {
		if (it->second == s)
			continue;
		DAPacket r(p);
		it->second->sendPacket(&r);
	}
	unlock();
}

void DataService::addMob(unsigned short map, unsigned short x, unsigned short y, unsigned short apr)
{
	Map *m = getMap(map);
	m->addEntity(new Mob(x, y, m, apr, 0));
}

void DataService::updateStoredItem(int charId, int itemId, int qty, short mod)
{
	Database::cm->lock();
	if (qty > 0)
		Database::cm->storageQtyChange(charId, itemId, mod, qty);
	else
		Database::cm->storageRemoveItem(charId, itemId, mod);
	Database::cm->unlock();
}

void DataService::storeItem(int charId, int itemId, int qty, short mod)
{
	Database::cm->lock();
	Database::cm->storageAddItem(charId, itemId, mod, qty);
	Database::cm->unlock();
}

//Try to enter the map near the coordinates given
//Returns false if the map wasn't enterable
//rad = 0 indicates that the player should always be entered exactly where given
bool DataService::tryChangeMap(Character *c, unsigned short mn, unsigned short x, unsigned short y, unsigned short rad)
{
	if (!maps.count(mn)) {
		return false; //Map doesn't exist
	}
	Map *old = c->getMap();

	//Cannot lock the data service while a map lock is held
	//TODO this is wrong, we need to hold the dataservice map before unlocking the old map,
	//so that the player is unable to complete a logout here
	old->unlock();
	lock();
	old->lock();
	old->removeEntity(c);
	Server::leaveMap(c->getSession());
	old->unlock();

	Map *m = maps[mn];
	c->setX(x);
	c->setY(y);
	c->setMap(m);
	m->lock();
	c->changedMap(m);
	m->addEntity(c, rad);
	unlock();

	m->unlock();
	old->lock();

	return true;
}

void DataService::enterMap(Character *c)
{
	if (!maps.count(c->getMapId())) {
		assert(maps.count(START_MAP));
		c->setMap(maps[START_MAP]);
		c->setX(START_X);
		c->setY(START_Y);
	}

	Map *mp = c->getMap();

	c->changedMap(mp);
	mp->addEntity(c);

}

void DataService::clearAll()
{
	using std::pair;
	assert(characters->size() == 0);
	std::for_each(maps.begin(), maps.end(), [&](pair<unsigned short,Map *> m) { delete m.second; });
	maps.clear();
}

int DataService::prepareLogin(std::string name, char const *pw, unsigned int ip)
{
	lock();
	//TODO use key as an extra check
	int r = characters->prepareLogin(name, pw, ip);
	unlock();
	return r;
}

void DataService::lock()
{
	assert(LockSet::addLock(-2));
	int r = pthread_mutex_lock(&onlineLock);
	assert(!r);
}

void DataService::unlock()
{
	assert(LockSet::removeLock(-2));
	int r = pthread_mutex_unlock(&onlineLock);
	assert(!r);
}

Character *DataService::getCharacter(std::string name, CharacterSession *who, unsigned int ip)
{
	lock();
	Character *c = characters->getCharacter(name, who, ip);
	unlock();
	return c;
}

void DataService::leaveMap(Character *c)
{
	Map *m = c->getMap();

	m->removeEntity(c);
	Server::leaveMap(c->getSession());
}

void DataService::freeCharacter(Character *c)
{
	Map *m = c->getMap();
	m->lock();
	leaveMap(c);
	m->unlock();

	leaveGroup(c);
	lock();
	characters->freeCharacter(c);
	c->freeOid();
	unlock();
}

void DataService::groupListMembers(IDataStream *dp, Character *c)
{
	lock();
	Group *g = c->getGroup();
	if (g) {
		char buffer[256];
		unsigned char size;
		size = snprintf(buffer, 256, "Group Members");
		auto gm = g->begin();
		buffer[size++] = 0xa;
		size += snprintf(buffer + size, 256 - size, "* %s", (*gm)->getName().c_str());
		buffer[size++] = 0xa;
		std::for_each(++gm, g->end(), [&](Character *m) {
			size += snprintf(buffer + size, 256 - size, "  %s", m->getName().c_str());
			buffer[size++] = 0xa;
		});
		size += snprintf(buffer + size, 256 - size, "Total %hu", (unsigned short)g->size());
		dp->appendString(size, buffer);
	}
	else {
		dp->appendString(0x11, "Adventuring alone");
	}
	unlock();
}

void DataService::groupDistributeExp(unsigned int exp, unsigned short level, int mobId, Entity *e)
{
	if (e->getType() == Entity::E_CHARACTER) {
		//TODO should use the groups level
		Character *c = (Character *) e;
		Character *rec[13];
		int maxLevel = 0, pLevel = 0, dist;
		short n = 0;

		Group *g = c->getGroup();
		if (!g) {
			maxLevel = c->getLevel();
			pLevel = c->getLevel();
			dist = 100;
			n = 1;
			rec[0] = c;
		}
		else {
			dist = 140;
			std::for_each(g->begin(), g->end(), [&](Character *m) {
				if (m->dist(c) <= NEARBY) {
					maxLevel = maxLevel > m->getLevel() ? maxLevel : m->getLevel();
					pLevel += m->getLevel();
					rec[n++] = m;
					dist += 10;
				}
			});
			assert(n);
			if (n < 2)
				dist = 100;
		}

		if (maxLevel >= level) {
			switch(maxLevel - level) {
			case 0:
			case 1:
			case 2:
			case 3:
				break;
			case 4:
			case 5:
				exp = (exp * 80 + 99) / 100;
				break;
			case 6:
			case 7:
			case 8:
				exp = (exp * 60 + 99) / 100;
				break;
			case 9:
			case 10:
			case 11:
				exp = (exp * 40 + 99) / 100;
				break;
			case 12:
			case 13:
			case 14:
				exp = (exp * 20 + 99) / 100;
				break;
			default:
				exp = 1;
				break;
			}
		}

		for (short i = 0; i < n; i++) {
			int mod = (dist * rec[i]->getLevel() + pLevel - 1) / pLevel;// + 100 * pLevel - 1;
			mod = mod < (105 - 5 * n) ? mod : (105 - 5 * n);
			rec[i]->gainExp(mod * exp / 100, mobId);
		}

	}
}

void DataService::sendWhisper(CharacterSession *s, std::string &recip, std::string &msg)
{
	if (recip.length() > MAX_NAME_LEN)
		return;
	char buffer[100];

	lock();
	auto it = characters->find(recip);
	if (it == characters->end()) {
		snprintf(buffer, 100, "%s is nowhere to be found.", recip.c_str());
		Server::sendMessage(s, buffer, 0);
	}
	else if (it->second->getCharacter()->getSettings() & Character::Settings::LISTEN_TO_WHISPERS) {
		snprintf(buffer, 100, "%s> %s", recip.c_str(), msg.c_str());
		Server::sendMessage(s, buffer, 0);
		snprintf(buffer, 100, "%s\" %s", s->getCharacter()->getName().c_str(), msg.c_str());
		Server::sendMessage(it->second, buffer, 0);
	}
	else {
		snprintf(buffer, 100, "%s can't hear you.", recip.c_str());
		Server::sendMessage(s, buffer, 0);
	}

	unlock();
}

void DataService::groupChat(CharacterSession *s, std::string &msg)
{
	lock();
	Group *g = s->getCharacter()->getGroup();
	if (g) {
		char buffer[100];
		snprintf(buffer, 100, "[!%s] %s", s->getCharacter()->getName().c_str(), msg.c_str());
		std::for_each(g->begin(), g->end(), [&](Character *c) {
			Server::sendMessage(c->getSession(), buffer, 0xb);
		});
	}
	else {
		Server::sendMessage(s, "You are not in a group.", 0);
	}
	unlock();
}

void DataService::guildChat(CharacterSession *s, std::string &msg)
{
	lock();
	Guild *g =s->getCharacter()->getGuild();
	if (g) {
		char buffer[100];
		snprintf(buffer, 100, "<!%s> %s", s->getCharacter()->getName().c_str(), msg.c_str());
		const std::vector<Character *> &members = g->getOnline();
		std::for_each(members.begin(), members.end(), [&](Character *c) {
			Server::sendMessage(c->getSession(), buffer, 0xc);
		});
	}
	else {
		Server::sendMessage(s, "You are not in a guild.", 0);
	}
	unlock();
}

Guild *DataService::createGuild(const char *name)
{
	lock();
	//validate name
	for (const char *n = name; *n; n++) {
		if (!(isalpha(*n) || *n == ' ')) {
			unlock();
			return 0;
		}
	}

	int gid = Database::createGuild(name);
	if (gid < 0) {
		unlock();
		return 0;
	}
	Guild *g = new Guild(gid, name);

	unlock();
	return g;
}

bool DataService::addGuildMember(Guild *g, Character *c, Guild::Rank rank)
{
	bool ret;

	lock();
	if (!g || c->getGuild())
		ret = false;
	else {
		g->addMember(c, rank, true);
		ret = true;
		Database::addToGuild(c->getId(), g->getId(), (short)rank);
	}
	unlock();

	return ret;
}

void DataService::blacklist(const char *name, int priv)
{
	lock();
	auto cs = characters->find(name);
	if (cs != characters->end()) {
		if (priv <= cs->second->getCharacter()->getPrivilegeLevel()) {
			unlock();
			throw E_INVALID;
		}
		int expTime = time(NULL) + 60*60*24*7;
		Database::banIp(cs->second->getIp(), expTime);
		Database::cm->lock();
		Database::cm->ban(name, expTime);
		Database::cm->unlock();
		addToBlacklists(cs->second->getIp(), expTime);
		cs->second->disconnect();
	}
	else {
		unlock();
		throw E_NOEXIST;
	}
	unlock();
}

void DataService::banChar(const char *name, int priv)
{
	lock();
	auto cs = characters->find(name);
	if (cs != characters->end()) {
		if (priv <= cs->second->getCharacter()->getPrivilegeLevel()) {
			unlock();
			throw E_INVALID;
		}
		int expTime = time(NULL) + 60*60*24;
		Database::cm->lock();
		Database::cm->ban(name, expTime);
		Database::cm->unlock();
		cs->second->disconnect();
	}
	else {
		unlock();
		throw E_NOEXIST;
	}
	unlock();
}

void DataService::gotoChar(const char *name, Character *c)
{
	lock();
	auto cs = characters->find(name);
	if (cs != characters->end()) {
		Character *t = cs->second->getCharacter();
		Map *m = c->getMap();
		m->lock();
		Server::leaveMap(c->getSession());
		m->removeEntity(c);
		m->unlock();
		m = t->getMap();
		m->lock();
		c->setMap(m);
		// TODO this probably wrong
		c->setX(t->getX());
		c->setY(t->getY());
		c->changedMap(m);
		m->addEntity(c, 3);
		m->unlock();
	}
	else {
		unlock();
		throw E_NOEXIST;
	}
	unlock();
}

void DataService::recallChar(const char *name, int priv, Character *c)
{
	lock();
	auto cs = characters->find(name);
	if (cs != characters->end()) {
		Character *t = cs->second->getCharacter();
		if (t->getPrivilegeLevel() > priv) {
			unlock();
			throw E_INVALID;
		}
		Map *m = t->getMap();
		m->lock();
		Server::leaveMap(cs->second);
		m->removeEntity(t);
		m->unlock();
		m = c->getMap();
		m->lock();
		t->setMap(m);
		t->changedMap(m);
		t->setX(c->getX());
		t->setY(c->getY());
		m->addEntity(t, 3);
		m->unlock();
	}
	else {
		unlock();
		throw E_NOEXIST;
	}
	unlock();
}

bool DataService::removeGuildMember(Guild *g, const char *charName)
{
	lock();
	bool ret = g->removeMember(charName);
	if (ret) {
		Database::removeFromGuild(charName);
	}
	unlock();

	return ret;
}

bool DataService::promoteGuildMember(Guild *g, const char *charName, Guild::Rank newRank)
{
	lock();
	bool ret = g->setRank(charName, newRank);
	if (ret) {
		Database::updateGuildRank(charName, newRank);
	}
	unlock();

	return ret;
}

void DataService::deleteGuild(Guild *g)
{
	lock();
	int gid = g->getId();
	delete g;
	Database::deleteGuild(gid);
	unlock();
}

void DataService::groupInvite(Character *m, std::string nm)
{
	lock();
	//lower(nm);
	auto it = characters->find(nm);
	if (it != characters->end()) {
		Character *c = it->second->getCharacter();
		if (c->canInvite()) {
			Server::groupInvite(c->getSession(), m->getName().size(), m->getName().c_str());
		}
		else if (c->getGroup()) {
			Server::sendMessage(m->getSession(), "Already a member of another group.");
		}
		else {
			Server::sendMessage(m->getSession(), "Refuses to join this group.");
		}
	}
	unlock();
}

void DataService::groupAccept(Character *m, std::string gn)
{
	lock();

	//can m join a group?
	if (!m->canInvite()) {
		unlock();
		return;
	}

	auto it = characters->find(gn);
	if (it != characters->end()) {
		Character *gm = it->second->getCharacter();
		Group *group = gm->getGroup();
		if (!group) {
			//create group
			group = new Group;
			group->push_back(gm);
			gm->setGroup(group);
		}
		if (group->size() < 13) {
			group->push_back(m);
			m->setGroup(group);
			char buffer[100];
			snprintf(buffer, 100, "%s is joining this group.", m->getName().c_str());
			std::for_each(group->begin(), group->end(), [&](Character *c) {
				Server::sendMessage(c->getSession(), buffer);
			});
		}
		else {
			Server::sendMessage(m->getSession(), "The group is full.");
		}
	}

	unlock();

}

void DataService::leaveGroup(Character *c)
{
	lock();
	Group *group = c->getGroup();
	if (group) {
		if (group->size() > 2) {
			char buffer[100];
			snprintf(buffer, 100, "%s has left the group.", c->getName().c_str());
			std::for_each(group->begin(), group->end(), [&](Character *m) {
				Server::sendMessage(m->getSession(), buffer);
			});
			group->remove(c);
			c->clearGroup();
		}
		else {
			auto gm = group->begin();
			Server::sendMessage((*gm)->getSession(), "Group disbanded.");
			(*(gm++))->clearGroup();
			Server::sendMessage((*gm)->getSession(), "Group disbanded.");
			(*gm)->clearGroup();
			delete group;
		}
	}
	unlock();
}

void DataService::broadcast(const char *msg)
{
	lock();
	for (std::pair<std::string, CharacterSession *> p : *characters) {
		Server::sendMessage(p.second, msg);
		//Server::talked(p.second, 0, 0, msg, strlen(msg));
	}
	unlock();
}

void DataService::tick()
{
	//TODO I think you might need to lock the data service before doing the maps
	std::vector<std::function<void ()> > deferred;

	lock();
	std::for_each(maps.begin(), maps.end(), [&](std::pair<unsigned short, Map *> m) {
		m.second->lock();
		m.second->tick(deferred);
		m.second->unlock();
	});
	unlock();

	for (std::function<void ()> fn : deferred)
		fn();

	infoTick = (infoTick + 1) % 30;
	if (infoTick == 0) {
		char data[5] = { 0, 5, 0, 1, 0x20 };
		DAPacket clearChat(Server::SYSTEM_MESSAGE, data, 5);
		sendToAll(&clearChat, Character::Settings::BELIEVE_IN_WISDOM);
	}
	else if (infoTick == 15) {
		lock(); //to use the characters list, probably a good idea to change

		std::for_each(characters->begin(), characters->end(), [&](std::pair<std::string, CharacterSession *> p) {
			p.second->getCharacter()->getMap()->lock();
			Database::cm->lock(); //to use the char manager
			Database::cm->saveCharacter(p.second->getCharacter());
			Database::cm->unlock();
			p.second->getCharacter()->getMap()->unlock();
		});

		Database::cm->lock();
		Database::cm->finish();
		Database::cm->unlock();
		unlock();
	}
	//Other recurring maintenance tasks belong here
}

std::vector<Character *> *DataService::getCountryList()
{
	lock();
	std::vector<Character *> *ret = characters->countryList();
	unlock();
	return ret;
}

void DataService::updateItem(int charId, unsigned short slot, int itemId, short qty, int dur, short mod, short idd)
{
	Database::cm->lock();
	//currently we can't find out if it is currently in the DB or not via query, so
	Database::cm->removeItem(charId, slot);
	Database::cm->addItem(charId, itemId, slot, mod, qty, dur, idd);
	Database::cm->unlock();
}

void DataService::removeItem(int charId, unsigned short slot)
{
	Database::cm->lock();
	Database::cm->removeItem(charId, slot);
	Database::cm->unlock();
}

void DataService::addQuest(int charId, int questId, int qp, int timer)
{
	Database::cm->lock();
	Database::cm->addQuest(charId, questId, qp, timer);
	Database::cm->unlock();
}

void DataService::updateQuest(int charId, int questId, int qp, int timer)
{
	Database::cm->lock();
	Database::cm->updateQuest(charId, questId, qp, timer);
	Database::cm->unlock();
}

void DataService::addLegend(int charId, int legendId, const char* textParam,
		int intParam, int timestamp)
{
	Database::cm->lock();
	Database::cm->addMark(charId, legendId, textParam, intParam, timestamp);
	Database::cm->unlock();
}

void DataService::updateLegend(int charId, int legendId, const char* textParam,
		int intParam, int timestamp)
{
	Database::cm->lock();
	Database::cm->updateMark(charId, legendId, textParam, intParam, timestamp);
	Database::cm->unlock();
}

void DataService::deleteLegend(int charId, int legendId)
{
	Database::cm->lock();
	Database::cm->deleteMark(charId, legendId);
	Database::cm->unlock();
}

void DataService::addSkill(int charId, int skillId, unsigned short slot)
{
	Database::cm->lock();
	Database::cm->addSkill(charId, skillId, slot);
	Database::cm->unlock();
}

void DataService::removeSkill(int charId, unsigned short slot)
{
	Database::cm->lock();
	Database::cm->removeSkill(charId, slot);
	Database::cm->unlock();
}

void DataService::moveSkill(int charId, unsigned short oldSlot,
		unsigned short newSlot)
{
	Database::cm->lock();
	Database::cm->moveSkill(charId, oldSlot, newSlot);
	Database::cm->unlock();
}

void DataService::swapSkills(int charId, unsigned short slotOne,
		unsigned short slotTwo, int idOne, int idTwo)
{
	Database::cm->lock();
	Database::cm->swapSkills(charId, slotOne, slotTwo, idOne, idTwo);
	Database::cm->unlock();
}

void DataService::addSecret(int charId, int skillId, unsigned short slot)
{
	Database::cm->lock();
	Database::cm->addSecret(charId, skillId, slot);
	Database::cm->unlock();
}

void DataService::removeSecret(int charId, unsigned short slot)
{
	Database::cm->lock();
	Database::cm->removeSecret(charId, slot);
	Database::cm->unlock();
}

void DataService::moveSecret(int charId, unsigned short oldSlot,
		unsigned short newSlot)
{
	Database::cm->lock();
	Database::cm->moveSecret(charId, oldSlot, newSlot);
	Database::cm->unlock();
}

void DataService::swapSecrets(int charId, unsigned short slotOne,
		unsigned short slotTwo, int idOne, int idTwo)
{
	Database::cm->lock();
	Database::cm->swapSecrets(charId, slotOne, slotTwo, idOne, idTwo);
	Database::cm->unlock();
}

void DataService::updatePath(int charId, unsigned short path,
		unsigned short pathmask)
{
	Database::cm->lock();
	Database::cm->updatePath(charId, path, pathmask);
	Database::cm->unlock();
}

void DataService::updateHairstyle(int charId, unsigned short style)
{
	Database::cm->lock();
	Database::cm->updateHairstyle(charId, style);
	Database::cm->unlock();
}

void DataService::updateHaircolor(int charId, unsigned short color)
{
	Database::cm->lock();
	Database::cm->updateHaircolor(charId, color);
	Database::cm->unlock();
}

void DataService::updateNation(int charId, unsigned short nation)
{
	Database::cm->lock();
	Database::cm->updateNation(charId, nation);
	Database::cm->unlock();
}
