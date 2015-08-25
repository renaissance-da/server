/*
 * script::bindings.cpp
 *
 *  Created on: 2012-12-17
 *      Author: per
 */


#include "npc_bindings.h"
#include "CharacterSession.h"
#include "defines.h"
#include "NPC.h"
#include "DataService.h"
#include "Map.h"
#include <pthread.h>
#include "ScriptTrigger.h"
#include "random.h"
#include "lower.h"
#include "char_bindings.h"
#include "entity_bindings.h"
#include "Mob.h"
#include "MobInfo.h"
#include "map_bindings.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

lua_State *L_scr = 0;
pthread_mutex_t scriptLock = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

void lockScripts()
{
	pthread_mutex_lock(&scriptLock);
}

void unlockScripts()
{
	pthread_mutex_unlock(&scriptLock);
}

//Send a message to the player
//correct arguments are session (light userdata),
//npc name(string), npc text(string),
//[number of options(number, must be smaller than 128)] {option(string)}
int script::sendMessage(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 3) {
		//TODO, error codes possibly
		return 0;
	}
	Character *c = (Character *)lua_touserdata(L, 1);
	CharacterSession *s = c->getSession();

	Entity *e = (Entity *)lua_touserdata(L,2);

	size_t len;
	const char *text = lua_tolstring(L, 3, &len);

	const char **opts;
	if (n > 3)
		opts = new const char*[n-3];
	else
		opts = 0;

	for (int i = 3; i < n; i++) {
		opts[i-3] = lua_tolstring(L, i+1, NULL);
	}

	//TODO parse options
	Server::npcDlg(s, e, text, len, n-3, opts);
	delete[] opts;

	return 0;
}

void initDlg(Character *c, NPC *npc)
{

	lockScripts();
	lua_getglobal(L_scr, "initDlg");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)npc);
	if (lua_pcall(L_scr, 3, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::initDlg: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	}
	unlockScripts();
}

void talkResponse(Character *c, NPC *npc, std::string &text)
{
	lockScripts();
	lua_getglobal(L_scr, "talkResponse");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)npc);
	lua_pushstring(L_scr, text.c_str());
	if (lua_pcall(L_scr, 4, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::talkResponse: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	}
	unlockScripts();
}

bool initScript(Character *c, ScriptTrigger *script)
{
	lockScripts();
	lua_getglobal(L_scr, "initScript");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)script);
	bool err = lua_pcall(L_scr, 3, 1, 0);
	if (err) {
		unlockScripts();
		LOG4CPLUS_ERROR(script::log, "Error in call to script::initScript: "
				<< lua_tostring(L_scr, -1));
		lua_pop(L_scr, 1);
		return true; // remove
	}

	bool res = lua_toboolean(L_scr, -1);
	lua_pop(L_scr, 1);
	unlockScripts();
	return res;
}

int scriptTimer(int timerId)
{
	lockScripts();
	lua_getglobal(L_scr, "runTimer");
	lua_pushnumber(L_scr, timerId);
	bool err = lua_pcall(L_scr, 1, 1, 0);
	if (err) {
		unlockScripts();
		LOG4CPLUS_ERROR(script::log, "Error in call to script::scriptTimer: "
				<< lua_tostring(L_scr, -1));
		lua_pop(L_scr, 1);
		return 0;
	}

	int next = lua_tonumber(L_scr, -1);
	lua_pop(L_scr, 1);
	unlockScripts();

	return next > 0 ? next + time(NULL) : next;
}

bool initScript(Character *c, ScriptTrigger *script, Item *itm)
{
	lockScripts();
	lua_getglobal(L_scr, "initScriptDrop");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)script);
	lua_pushnumber(L_scr, (double)itm->getId());
	bool err = lua_pcall(L_scr, 4, 1, 0);
	if (err) {
		unlockScripts();
		LOG4CPLUS_ERROR(script::log, "Error in call to script::initScript: "
				<< lua_tostring(L_scr, -1));
		lua_pop(L_scr, 1);
		return true; // Delete this trigger
	}

	bool ret = lua_toboolean(L_scr, -1);
	lua_pop(L_scr, 1);
	unlockScripts();
	return ret;
}

void resumeDlg(Character *c, Entity *e, unsigned short opt)
{
	lockScripts();
	lua_getglobal(L_scr, "resumeDlg");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)e);
	lua_pushnumber(L_scr, (double)opt);
	if (lua_pcall(L_scr, 4, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::resumeDlg: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	}
	unlockScripts();
}

void resumeDlg(Character *c, Entity *e, std::string &rep)
{
	lockScripts();
	lua_getglobal(L_scr, "resumeDlg");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushlightuserdata(L_scr, (void *)e);
	lua_pushstring(L_scr, rep.c_str());
	if (lua_pcall(L_scr, 4, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::resumeDlg: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	}
	unlockScripts();
}

void initScripts()
{
	lockScripts();

	if (L_scr)
		lua_close(L_scr);

	L_scr = luaL_newstate();
	luaL_openlibs(L_scr);
	lua_State *L = L_scr;
	using namespace script;
	REGISTER(sendMessage);
	REGISTER(getTriggerId);
	REGISTER(addnpc);
	REGISTER(sendVendList);
	REGISTER(itemOnList);
	REGISTER(showSellList);
	REGISTER(offerSkills);
	REGISTER(skillCost);
	REGISTER(sysmsg);
	REGISTER(closeDialog);
	REGISTER(longDialog);
	REGISTER(upgrade);
	REGISTER(addtrigger);
	REGISTER(queryText);
	REGISTER(showStorage);
	REGISTER(getNearbyPlayer);
	REGISTER(getNearByOid);
	REGISTER(getEquipSlot);
	REGISTER(canCustomize);
	REGISTER(forgetSkills);
	REGISTER(showRepairList);
	REGISTER(queryTextCustom);
	REGISTER(spawnNewMob);
	REGISTER(broadcast);
	REGISTER(teleportInstance);

	script::loadEntityBindings(L_scr);
	script::loadCharBindings(L_scr);
	script::loadMapBindings(L_scr);

	int err = luaL_dofile(L_scr, "npc/scripts.lua");
	
	if (err)
	    LOG4CPLUS_ERROR(script::log, "Error while loading scripts.");

#ifdef WIN32
	lua_getglobal(L_scr, "initNpcsDir");
#else
	lua_getglobal(L_scr, "initNpcs");
#endif
	lua_call(L_scr, 0, 0);

	unlockScripts();

}

int script::teleportInstance(lua_State *L)
{
	if (lua_gettop(L) < 4) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int mapid = lua_tonumber(L, 2);
	short x = lua_tonumber(L, 3);
	short y = lua_tonumber(L, 4);

	Group *g = c->getGroup();
	if (!g) {
		c->sendMessage("You are not in a group.");
		return 0;
	}
	bool isNew;
	Instance *i = g->getInstance(mapid, &isNew);

	if (!DataService::getService()->tryChangeMap(c, i->getId(), x, y, 3)) {
		LOG4CPLUS_WARN(script::log, "NPC teleportInstance failed to warp player "
			       << c->getName() << " to map " << mapid << ": " << x
			       << " " << y << ".");
	}

	if (isNew) {
		lua_pushinteger(L, i->getId());
		return 1;
	}
	return 0;
}

int script::forgetSkills(lua_State *L)
{
	if (lua_gettop(L) < 4)
		return 0;

	size_t len;
	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	const char *text = lua_tolstring(L, 3, &len);
	bool secrets = lua_toboolean(L, 4);

	Server::sendForgetList(c->getSession(), e, (unsigned short)len, text, secrets);
	return 0;
}

int script::broadcast(lua_State *L)
{
	if (lua_gettop(L) < 1) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	DataService::getService()->broadcast(lua_tostring(L, 1));
	return 0;
}

int script::sysmsg(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	const char *msg = lua_tolstring(L, 2, 0);
	Server::sendMessage(c->getSession(), msg);
	return 0;
}

int script::upgrade(lua_State *L)
{
	//charactersession, type(0=all), rate
	//returns true on success
	//TODO not sure how this one should work for final version
	const int n = lua_gettop(L);
	if (n != 3)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	int type = (int)lua_tonumber(L, 2);
	int rate = (int)lua_tonumber(L, 3);

	if (rate < 100 && ((random() % 100) >= rate)) {
		return 0;
	}
	//here im assuming this is the upgrader's function, but what if it should work
	//for players too?
	Item *itm = c->getInventory()[0];
	if (!itm || itm->getType() != BaseItem::EQUIP)
		return 0;
	Equipment *eq = (Equipment*) itm;
	if (!eq->canModify() || (eq->getSlot() != Equipment::WEAPON && eq->getSlot() != Equipment::ARMOR)
			|| eq->getMod() > Equipment::GRAND) {
		return 0;
	}

	if (c->countItems(eq->getId(), eq->getMod()) < 3) {
		return 0;
	}

	//finally
	Equipment *neweq = (Equipment*) new Equipment(eq->getId(), eq->getQty(), eq->getMaxDur());
	neweq->setMod(eq->getMod() + 1);
	Item *r = c->getInventory().take(eq->getId(), 3, eq->getMod(), c);
	if (r) {
		c->getItem(neweq);
		delete r;
	}
	lua_pushboolean(L, true);
	return 1;
}

int script::addtrigger(lua_State *L)
{
	if (lua_gettop(L) != 4)
		return 0;

	int map = (int)lua_tonumber(L, 1);
	short x = (short)lua_tonumber(L, 2);
	short y = (short)lua_tonumber(L, 3);
	int lifetime = (int)lua_tonumber(L, 4);

	Map *m = DataService::getService()->getMap(map);
	if (!m) {
		return luaL_error(L, "In %s: Failed to add trigger because map %d doesn't exist.",
				  __PRETTY_FUNCTION__, map);
	}
	ScriptTrigger *t = new ScriptTrigger(x, y, m, lifetime);
	lua_pushnumber(L, (double)t->getId());
	m->addTrigger(t);
	return 1;
}

bool runEffect(Character *c, int effectId)
{
	lockScripts();
	lua_getglobal(L_scr, "runEffect");
	lua_pushnumber(L_scr, (double)(c->getId()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushnumber(L_scr, (double)effectId);
	bool res = false, err = lua_pcall(L_scr, 3, 1, 0);

	if (!err) {
		res = lua_toboolean(L_scr, -1);
		lua_pop(L_scr, 1);
	}
	unlockScripts();

	return res || err;
}

bool doSpell(Character *c, int spellId)
{
	lockScripts();
	lua_getglobal(L_scr, "doSpell");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushnumber(L_scr, (double)spellId);
	if (lua_pcall(L_scr, 3, 1, 0) != 0) {
	    unlockScripts();
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::doSpell: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	    return false;
	}
	bool ret = lua_toboolean(L_scr, 1);
	lua_pop(L_scr, 1);
	unlockScripts();

	return ret;
}

void script::recall(Character *c, int mapid, short x, short y)
{
	lockScripts();
	lua_getglobal(L_scr, "recall");
	lua_pushnumber(L_scr, (double)(c->getOid()));
	lua_pushlightuserdata(L_scr, (void *)c);
	lua_pushnumber(L_scr, (double)mapid);
	lua_pushnumber(L_scr, (double)x);
	lua_pushnumber(L_scr, (double)y);
	if (lua_pcall(L_scr, 5, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::recall: "
			    << lua_tostring(L_scr, -1));
	    lua_pop(L_scr, 1);
	}
	unlockScripts();
}

int script::longDialog(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 6) {
		return 0;
	}
	size_t textLen;
	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	const char *text = lua_tolstring(L, 3, &textLen);
	char pos = (char)lua_tonumber(L, 4);
	bool pre = lua_toboolean(L, 5);
	bool post = lua_toboolean(L, 6);

	Server::sendLongDlg(c->getSession(), e, text, textLen, post, pre, pos, 0, 0);
	return 0;
}

int script::getTriggerId(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1) {
		return 0;
	}

	assert(lua_type(L, 1) == LUA_TLIGHTUSERDATA);
	ScriptTrigger *sc = (ScriptTrigger *)lua_touserdata(L, 1);

	lua_pushnumber(L, (double)sc->getId());
	return 1;
}

int script::addnpc(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 6) {
		return 0;
	}

	unsigned short map = (unsigned short)lua_tonumber(L, 1);
	unsigned short x = (unsigned short)lua_tonumber(L, 2);
	unsigned short y = (unsigned short)lua_tonumber(L, 3);
	const char *name = lua_tolstring(L, 4, 0);
	unsigned short apr = (unsigned short)lua_tonumber(L, 5);
	unsigned short dir = (unsigned short)lua_tonumber(L, 6);

	Map *m = DataService::getService()->getMap(map);
	if (!m)
		return 0;

	NPC *npc = new NPC(x, y, m, name, apr, dir);
	m->addEntity(npc);

	lua_pushnumber(L, (double)npc->getOid());
	return 1;

}

int script::sendVendList(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 3)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t sz;
	const char *text = lua_tolstring(L, 3, &sz);
	std::vector<BaseItem *> items;
	for (int i = 3; i < n; i++) {
		BaseItem *bi = BaseItem::getById((unsigned int)(lua_tonumber(L, i+1)));
		if (bi)
			items.push_back(bi);
		else
			return luaL_error(L, "An Entity labeled as %s tried to sell an item (id=%d) which wasn't loaded\n",
					  e->getName().c_str(), (unsigned int)(lua_tonumber(L, i+1)));
	}
	Server::sendVendList(c->getSession(), e, sz, text, items);
	return 0;
}

int script::itemOnList(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1)
		return 0;

	const char *item = lua_tolstring(L, 1, NULL);
	for (int i = 1; i < n; i++) {
		BaseItem *bi = BaseItem::getById((int)lua_tonumber(L, i+1));
		if (!strcmp(bi->getName(), item)) {
			lua_pushnumber(L, (double)bi->getId());
			return 1;
		}
	}
	return 0;
}

int script::showSellList(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t sz;
	const char *text = lua_tolstring(L, 3, &sz);

	std::vector<char> slots;
	Item **items = c->getInventory().allItems();

	if (n > 3) {
		for (int i = 3; i < n; i++) {
			int itemId = (int)lua_tonumber(L, i+1);
			for (int j = 0; j < NUM_ITEMS; j++) {
				if (items[j] && items[j]->getId() == itemId)
					slots.push_back((char)(j+1));
			}
		}
	}
	else {
		for (int j = 0; j < NUM_ITEMS; j++) {
			if (items[j])
				slots.push_back((char)(j+1));
		}
	}

	Server::sendItemSublist(c->getSession(), e, sz, text, slots);
	return 0;
}

int script::showRepairList(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	std::vector<char> slots;
	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t sz;
	const char *text = lua_tolstring(L, 3, &sz);

	Item **items = c->getInventory().allItems();

	for (int j = 0; j < NUM_ITEMS; j++) {
		if (items[j] && items[j]->canRepair()) {
			slots.push_back((char)j+1);
		}
	}

	Server::sendItemSublist(c->getSession(), e, sz, text, slots);
	return 0;
}

int script::queryText(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t len;
	const char *text = lua_tolstring(L, 3, &len);

	Server::queryAmount(c->getSession(), e, len, text);

	return 0;
}

int script::queryTextCustom(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 4)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t len, qlen;
	const char *text = lua_tolstring(L, 3, &len);
	const char *query = lua_tolstring(L, 4, &qlen);

	Server::queryCustom(c->getSession(), e, len, text, qlen, query);

	return 0;
}

int script::offerSkills(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 4)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t sz;
	const char *text = lua_tolstring(L, 3, &sz);
	bool spells = lua_toboolean(L, 4);

	std::vector<SkillInfo *> skills;
	for (int i = 5; i <= n; i++) {
		SkillInfo *si = SkillInfo::getById((unsigned)lua_tonumber(L, i));
		if (si)
			skills.push_back(si);
	}

	Server::sendSkillList(c->getSession(), e, sz, text, skills, spells);
	return 0;
}

int script::skillCost(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	int id = (int)lua_tonumber(L, 3);
	SkillInfo *si = SkillInfo::getById(id);
	if (!si)
		return 0;

	int nItems = 0;
	int itemAmt[3];
	char item[3][64];
	if (si->item1) {
		nItems++;
		BaseEquipment *itm = (BaseEquipment*)BaseItem::getById(si->item1);
		if (si->item1mod)
			snprintf(item[0], 64, "%s %s", Equipment::getModName(itm->getSlot(), si->item1mod), itm->getName());
		else
			snprintf(item[0], 64, "%s", itm->getName());
		itemAmt[0] = si->item1Qty;
	}
	if (si->item2) {
		BaseEquipment *itm = (BaseEquipment*)BaseItem::getById(si->item2);
		if (si->item2mod)
			snprintf(item[nItems], 64, "%s %s", Equipment::getModName(itm->getSlot(), si->item2mod), itm->getName());
		else
			snprintf(item[nItems], 64, "%s", itm->getName());
		itemAmt[nItems] = si->item2Qty;
		nItems++;
	}
	if (si->item3) {
		BaseEquipment *itm = (BaseEquipment*)BaseItem::getById(si->item3);
		if (si->item3mod)
			snprintf(item[nItems], 64, "%s %s", Equipment::getModName(itm->getSlot(), si->item3mod), itm->getName());
		else
			snprintf(item[nItems], 64, "%s", itm->getName());
		itemAmt[nItems] = si->item3Qty;
		nItems++;
	}
	char buffer[256];
	switch (nItems) {
	case 0:
		snprintf(buffer, 256, "To learn this ability, %d coins are required.", si->gold);
		break;
	case 1:
		snprintf(buffer, 256, "To learn this ability, %d coins and %s(%d) are required.", si->gold, item[0], itemAmt[0]);
		break;
	case 2:
		snprintf(buffer, 256, "To learn this ability, %d coins, %s(%d), and %s(%d) are required.",
				si->gold, item[0], itemAmt[0], item[1], itemAmt[1]);
		break;
	case 3:
	default:
		snprintf(buffer, 256, "To learn this ability, %d coins, %s(%d), %s(%d), and %s(%d) are required.",
				si->gold, item[0], itemAmt[0], item[1], itemAmt[1], item[2], itemAmt[2]);
		break;
	}

	const char *opts[2] = { "Yes", "No" };

	Server::npcDlg(c->getSession(), e, buffer, strlen(buffer),
			2, opts);
	return 0;
}

int script::closeDialog(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);

	Server::closeDialog(c->getSession());
	return 0;
}

int script::showStorage(lua_State *L)
{
	if (lua_gettop(L) < 3) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	Entity *e = (Entity *)lua_touserdata(L, 2);
	size_t textLen;
	const char *text = lua_tolstring(L, 3, &textLen);

	Server::sendVendList(c->getSession(), e, textLen, text, c->getStorage());
	return 0;
}

int script::getNearbyPlayer(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}
	Character *c = (Character *)lua_touserdata(L, 1);
	const char *name = lua_tostring(L, 2);

	std::vector<Entity *> nearby;
	c->getMap()->getNearbyEntities(c, nearby);
	std::string sname(name);
	lower(sname);

	for (Entity *e : nearby) {
		if (e == c || e->getType() != Entity::E_CHARACTER)
			continue;
		std::string lname = e->getName();
		lower(lname);
		if (lname.compare(sname) == 0) {
			lua_pushlightuserdata(L, e);
			lua_pushnumber(L, e->getOid());
			return 2;
		}
	}

	return 0;
}

int script::getNearByOid(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}
	Character *c = (Character *)lua_touserdata(L, 1);
	int oid = lua_tonumber(L, 2);

	Entity *r = c->getMap()->getNearByOid(c, oid, NEARBY);
	if (r && r->getType() == Entity::E_CHARACTER) {
		lua_pushlightuserdata(L, r);
		return 1;
	}
	return 0;
}

int script::getEquipSlot(lua_State *L)
{
	if (lua_gettop(L) < 1) {
		return 0;
	}

	int id = lua_tonumber(L, 1);
	BaseItem *bi = BaseItem::getById(id);
	if (bi->getType() == BaseItem::EQUIP)
		lua_pushnumber(L, ((BaseEquipment*)bi)->getSlot());
	else
		lua_pushnumber(L, 0);
	return 1;
}

int script::canCustomize(lua_State *L)
{
	if (lua_gettop(L) < 1) {
		return 0;
	}

	int id = lua_tonumber(L, 1);
	BaseItem *bi = BaseItem::getById(id);
	if (bi->getType() == BaseItem::EQUIP)
		lua_pushboolean(L, ((BaseEquipment*)bi)->canModify());
	else
		lua_pushboolean(L, false);
	return 1;
}
