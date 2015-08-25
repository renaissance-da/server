/*
 * CharBindings.cpp
 *
 *  Created on: 2013-08-29
 *      Author: per
 */

#include "char_bindings.h"
#include "Character.h"
#include "npc_bindings.h"
#include "log4cplus/loggingmacros.h"
#include "DataService.h"
#include "lower.h"

void script::loadCharBindings(lua_State *L)
{
	REGISTER(baseHp);
	REGISTER(baseMp);
	REGISTER(isMaster);
	REGISTER(setMaster);
	REGISTER(addMaxHp);
	REGISTER(addMaxMp);
	REGISTER(isPure);
	REGISTER(rededicate);
	REGISTER(curWeight);
	REGISTER(incStat);
	REGISTER(forgetSkill);
	REGISTER(getPath);
	REGISTER(getGender);
	REGISTER(isDead);
	REGISTER(countLabor);
	REGISTER(getQuestProgress);
	REGISTER(getQuestTimer);
	REGISTER(getEquip);
	REGISTER(countItems);
	REGISTER(countKills);
	REGISTER(setPath);
	REGISTER(countItemId);
	REGISTER(maxStack);
	REGISTER(getItemName);
	REGISTER(repair);
	REGISTER(giveSkill);
	REGISTER(setHairstyle);
	REGISTER(setHairColor);
	REGISTER(addTracker);
	REGISTER(delTracker);
	REGISTER(giveExp);
	REGISTER(takeExp);
	REGISTER(sellItem);
	REGISTER(getSaleValue);
	REGISTER(buyItem);
	REGISTER(canSellItem);
	REGISTER(getSkillSublist);
	REGISTER(revive);
	REGISTER(teleport);
	REGISTER(chargeLabor);
	REGISTER(charge);
	REGISTER(giveItem);
	REGISTER(setQuestProgress);
	REGISTER(setQuestTimer);
	REGISTER(removeSkill);
	REGISTER(changeAttr);
	REGISTER(getRepairCost);
	REGISTER(repairAll);
	REGISTER(addLegendMark);
	REGISTER(updateLegendQty);
	REGISTER(withdrawItem);
	REGISTER(canDeposit);
	REGISTER(chargeGold);
	REGISTER(depositItem);
	REGISTER(storageInfo);
	REGISTER(depositGold);
	REGISTER(withdrawGold);
	REGISTER(countStoredGold);
	REGISTER(getLegendText);
	REGISTER(removeLegend);
	REGISTER(getItemId);
	REGISTER(getSkillLevel);
	REGISTER(getLegendTimestamp);
	REGISTER(inGuild);
	REGISTER(createGuild);
	REGISTER(addMember);
	REGISTER(removeMember);
	REGISTER(listMembers);
	REGISTER(deleteGuild);
	REGISTER(promoteMember);
}

inline Character *luaGetCharacter(lua_State *L)
{
	if (lua_gettop(L) < 1)
	    return 0;

	return (Character *)lua_touserdata(L, 1);
}

int script::baseHp(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	lua_pushnumber(L, c->getBaseStats()->getHp());
	return 1;
}

int script::baseMp(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	lua_pushnumber(L, c->getBaseStats()->getMp());
	return 1;
}

int script::isMaster(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	lua_pushboolean(L, c->isMaster());
	return 1;
}

int script::isPure(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	const int basePaths = c->getPathMask() & (paths[Master].mask - 1);
	lua_pushboolean(L, (basePaths ^ paths[c->getBasePath()].mask) == 0);
	return 1;
}

int script::setMaster(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (c)
		c->setMaster();

	return 0;
}

int script::addMaxHp(lua_State *L)
{
	if (lua_gettop(L) < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	int amt = lua_tonumber(L, 2);

	c->addMaxHp(amt);
	lua_pushboolean(L, true);
	return 1;
}

int script::addMaxMp(lua_State *L)
{
	if (lua_gettop(L) < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	int amt = lua_tonumber(L, 2);

	c->addMaxMp(amt);
	lua_pushboolean(L, true);
	return 1;
}

int script::rededicate(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	Path p = (Path)lua_tonumber(L, 2);
	c->rededicate(p);

	return 0;
}

int script::curWeight(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	lua_pushnumber(L, c->curWgt());
	return 1;
}

int script::incStat(lua_State *L)
{
	if (lua_gettop(L) < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	int stat = lua_tonumber(L, 2);
	bool canInc;
	switch(stat) {
	case 1:
		if (c->getBaseStats()->getStr() >= paths[c->getBasePath()].maxStr)
			canInc = false;
		else {
			c->incStr(1);
			canInc = true;
		}
		break;
	case 2:
		if (c->getBaseStats()->getDex() >= paths[c->getBasePath()].maxDex)
			canInc = false;
		else {
			c->incDex(1);
			canInc = true;
		}
		break;
	case 3:
		if (c->getBaseStats()->getInt() >= paths[c->getBasePath()].maxInt)
			canInc = false;
		else {
			c->incInt(1);
			canInc = true;
		}
		break;
	case 4:
		if (c->getBaseStats()->getWis() >= paths[c->getBasePath()].maxWis)
			canInc = false;
		else {
			c->incWis(1);
			canInc = true;
		}
		break;
	case 5:
		if (c->getBaseStats()->getCon() >= paths[c->getBasePath()].maxCon)
			canInc = false;
		else {
			c->incCon(1);
			canInc = true;
		}
		break;
	default:
		canInc = false;
		break;
	}

	lua_pushboolean(L, canInc);
	return 1;
}

int script::forgetSkill(lua_State *L)
{
	if (lua_gettop(L) < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = luaGetCharacter(L);
	if (!c)
		return 0;

	int slot = (int)lua_tonumber(L, 2);
	bool secret = lua_toboolean(L, 3);
	if (secret)
		c->forgetSecret(slot);
	else
		c->forgetSkill(slot);

	return 0;
}

int script::getPath(lua_State *L)
{
	int n = lua_gettop(L);

	if (n < 1)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	assert(lua_type(L,1) == LUA_TLIGHTUSERDATA);
	Character *c = (Character *)lua_touserdata(L, 1);
	lua_pushnumber(L, (double)c->getBasePath());

	return 1;
}

int script::getGender(lua_State *L)
{
	if (lua_gettop(L) != 1)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	lua_pushstring(L, c->getGender());
	return 1;
}

int script::isDead(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 1)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	lua_pushboolean(L, c->isDead());
	return 1;
}

int script::countLabor(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned short laborAmt;
	unsigned int laborTime;
	c->getLaborCountTime(laborAmt, laborTime);

	lua_pushnumber(L, (double)laborAmt);
	return 1;
}

int script::getQuestProgress(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int questId = (int)lua_tonumber(L, 2);

	lua_pushnumber(L, (double)c->getQuestProgress(questId));
	return 1;
}

int script::getQuestTimer(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	int questId = (int)lua_tonumber(L, 2);

	lua_pushnumber(L, (double)c->getQuestTimer(questId));
	return 1;
}

int script::getEquip(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	int slot = (int)lua_tonumber(L, 2);
	if (slot > NUM_EQUIPS) {
		return luaL_error(L, "Invalid equipment slot %d passed in call to %s.", slot, __PRETTY_FUNCTION__);
	}
	if (c->getEquipment()[slot]) {
		lua_pushnumber(L, c->getEquipment()[slot]->getId());
		return 1;
	}
	return 0;
}

int script::countItems(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned short slot = (unsigned short)(lua_tonumber(L, 2));

	lua_pushnumber(L, (double)c->getItemQty(slot));
	return 1;
}

int script::countKills(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	Character *c = (Character *)lua_touserdata(L, 1);
	int trackerId = (int)lua_tonumber(L, 2);

	if (n == 2)
		lua_pushnumber(L, (double)c->countAllKills(trackerId));
	else {
		int mobId = (int)lua_tonumber(L, 3);
		lua_pushnumber(L, (double)c->countKills(trackerId, mobId));
	}

	return 1;
}

int script::setPath(lua_State *L)
{
	int n = lua_gettop(L);

	if (n < 2)
	    return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);

	assert(lua_type(L,1) == LUA_TLIGHTUSERDATA);
	Character *c = (Character *)lua_touserdata(L, 1);
	assert(lua_type(L, 2) == LUA_TNUMBER);
	double p = lua_tonumber(L, 2);

	c->setPath((int)p);

	return 0;
}

int script::countItemId(lua_State *L)
{
	const int n = lua_gettop(L);

	if (n < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	assert(lua_type(L, 1) == LUA_TLIGHTUSERDATA);
	Character *c = (Character *)lua_touserdata(L, 1);
	assert(lua_type(L, 2) == LUA_TNUMBER);
	int itemId = (int)lua_tonumber(L, 2);
	unsigned short mod = 0;
	if (n >= 3) {
		mod = (unsigned short)lua_tonumber(L, 3);
	}

	lua_pushnumber(L, c->countItems(itemId, mod));
	return 1;
}

int script::maxStack(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int itemId = (int)lua_tonumber(L, 2);

	BaseItem *bi = BaseItem::getById(itemId);
	if (!bi) {
	    return luaL_error(L, "Invalid item ID (%d) passed to %s.", itemId, __PRETTY_FUNCTION__);
	}

	unsigned short max = bi->getMaxStack();
	if (max > 1) {
		unsigned short carried = c->countItems(itemId, 0);
		lua_pushnumber(L, max - carried);
	}
	else {
		lua_pushnumber(L, 0);
	}

	return 1;
}

int script::removeSkill(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int skid = (unsigned int)lua_tonumber(L, 2);

	lua_toboolean(L, c->removeSkill(skid));
	return 1;
}

int script::changeAttr(lua_State *L)
{
	if (lua_gettop(L) != 6)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	int str, dex, con, int_, wis;
	str = (int)lua_tonumber(L, 2);
	dex = (int)lua_tonumber(L, 3);
	con = (int)lua_tonumber(L, 4);
	int_ = (int)lua_tonumber(L, 5);
	wis = (int)lua_tonumber(L, 6);

	c->incStr(str);
	c->incDex(dex);
	c->incCon(con);
	c->incInt(int_);
	c->incWis(wis);

	return 0;
}

//TODO move to entity
int script::giveExp(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int reward = (unsigned int)lua_tonumber(L, 2);

	c->gainExp(reward);
	return 0;
}

//TODO move to entity
int script::takeExp(lua_State *L)
{
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int amt = (unsigned int)lua_tonumber(L, 2);

	lua_pushboolean(L, c->takeExp(amt));
	return 1;
}

int script::addTracker(lua_State *L)
{
	const int n = lua_gettop(L);

	if (n < 3) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}
	Character *c = (Character *)lua_touserdata(L, 1);
	int trackerId = (int)lua_tonumber(L, 2);
	int *mobIds = new int[n-2];
	for (int m = 0; m < n - 2; m++) {
		mobIds[m] = (int)lua_tonumber(L, m + 3);
	}
	c->addTracker(trackerId, mobIds, n-2);

	lua_pushboolean(L, true);
	return 1;
}

int script::delTracker(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int trackerId = (int)lua_tonumber(L, 2);
	c->deleteTracker(trackerId);

	return 0;
}

int script::setQuestProgress(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 3)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int questId = (int)lua_tonumber(L, 2);
	int progress = (int)lua_tonumber(L, 3);

	c->setQuestProgress(questId, progress);
	return 0;
}

int script::setQuestTimer(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 3)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int questId = (int)lua_tonumber(L, 2);
	int timer = (int)lua_tonumber(L, 3);

	c->setQuestTimer(questId, timer);
	return 0;
}

int script::revive(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 1)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	c->alive();
	return 0;
}

int script::teleport(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 4)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int map = (int)lua_tonumber(L, 2);
	short x = (short)lua_tonumber(L, 3);
	short y = (short)lua_tonumber(L, 4);

	if (!DataService::getService()->tryChangeMap(c, map, x, y, 3)) {
		LOG4CPLUS_WARN(script::log, "NPC teleport faild to warp player "
			       << c->getName() << " to map " << map << ": "
			       << x << " " << y << ".");
	}

	return 0;
}

int script::getLegendText(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int legendId = lua_tonumber(L, 2);
	const char *legendText = c->getLegendTextParam(legendId);

	if (legendText) {
		lua_pushstring(L, legendText);
		return 1;
	}
	return 0;
}

int script::addLegendMark(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 4) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int legendId = lua_tonumber(L, 2);
	const char *textParam = lua_tostring(L, 3);
	int intParam = lua_tonumber(L, 4);

	if (n >= 5) {
		int time = lua_tonumber(L, 5);
		c->addLegendItem(legendId, textParam, intParam, time);
	}
	else
		c->addLegendItem(legendId, textParam, intParam);

	return 0;
}

int script::getLegendTimestamp(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int legendId = lua_tonumber(L, 2);

	int timestamp = c->getLegendTimestamp(legendId);
	if (timestamp) {
		lua_pushnumber(L, timestamp);
		return 1;
	}
	return 0;
}

int script::updateLegendQty(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int legendId = lua_tonumber(L, 2);
	int intParam = lua_tonumber(L, 3);

	if (n == 3)
		c->changeLegendQty(legendId, intParam);
	else {
		const char *newText = lua_tostring(L, 4);
		c->changeLegendQty(legendId, intParam, newText);
	}
	return 0;
}

int script::removeLegend(lua_State *L)
{
	if (lua_gettop(L) < 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int legendId = lua_tonumber(L, 2);

	c->removeLegend(legendId);

	return 0;
}

int script::charge(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n % 2 == 0 || n < 1)
		return 0;
	else if (n == 1) {
		lua_pushboolean(L, true);
		return 1;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int *itms = new int[2*(n/2)];
	for (int i = 2; i < n; i += 2) {
		int id = (int)lua_tonumber(L, i);
		int amt = (int)lua_tonumber(L, i+1);
		if (c->countItems(id, 0) < amt) {
			delete[] itms;
			lua_pushboolean(L, false);
			return 1;
		}
		itms[i-2] = id;
		itms[i-1] = amt;
		//Note, assuming that the input doesn't contain duplicates here
	}

	std::vector<Item *> fallback;

	for (int i = 0; i < n/2; i++) {
		Item *ni = c->getInventory().take(itms[i<<1], itms[(i<<1) | 1], 0, c);
		if (!ni) {
			//Failed to take an item
			for (Item * itm : fallback)
				if (!c->getItem(itm))
					c->lostItem(itm);
			delete[] itms;
			return 0;
		}

		fallback.push_back(ni);
	}
	delete[] itms;

	for (Item *itm : fallback)
		delete itm;

	lua_pushboolean(L, true);
	return 1;

}
int script::giveItem(lua_State *L)
{
	if (lua_gettop(L) < 3)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	int id = (int)lua_tonumber(L, 2);
	int qty = (int)lua_tonumber(L, 3);

	BaseItem *bi = BaseItem::getById(id);
	if (!bi) {
		return luaL_error(L, "A script tried to give an item that wasn't defined (id=%d).", id);
	}

	Item *itm = new Item(bi);
	int times = 1;
	if (itm->getMaxQty())
		itm->setQty(qty);

	if (lua_gettop(L) == 4) {
		int mod = (int)lua_tonumber(L, 4);
		if (itm->getType() == BaseItem::EQUIP)
			((Equipment *)itm)->setMod(mod);
		else
		    LOG4CPLUS_WARN(script::log, "A script tried to set a modifier on a non-equipment item.");
	}

	bool success = c->getItem(itm);
	if (!success)
		delete itm;
	lua_pushboolean(L, success);
	return 1;
}

int script::chargeLabor(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned short amt = (unsigned short)lua_tonumber(L, 2);

	lua_pushboolean(L, c->useLabor(amt));
	return 1;
}

int script::getRepairCost(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 1)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);

	if (n == 1) {
		int cost = c->repairAllCost();
		if (cost == -1)
			return 0;
		lua_pushnumber(L, cost);
	}

	else
	{
		unsigned int slot = (unsigned int)lua_tonumber(L, 2);
		if (slot >= NUM_ITEMS)
			return 0;

		Item *itm = c->getInventory()[slot];
		if (!itm)
			return 0;
		lua_pushnumber(L, itm->repairCost());
	}

	return 1;
}

int script::getItemName(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int slot = (unsigned int)lua_tonumber(L, 2);

	if (slot >= NUM_ITEMS)
		return 0;

	Item *itm = c->getInventory()[slot];
	if (!itm)
		return 0;
	lua_pushlstring(L, itm->getName(), itm->getNameLen());
	return 1;
}

int script::repairAll(lua_State *L)
{
	if (lua_gettop(L) < 1)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);

	lua_pushboolean(L, c->repairAll());
	return 1;
}

int script::sellItem(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int id = (int)(lua_tonumber(L, 2));
	unsigned short amount = 1;
	if (n >= 3)
		amount = (unsigned short)lua_tonumber(L, 3);

	BaseItem *bi = BaseItem::getById(id);
	if (!c || !bi) {
		//r = false;
		lua_pushstring(L, "Sorry, what were we just talking about?");
		return 1;
	}
	else if (c->getGold() < bi->getVal()*amount) {
		lua_pushstring(L, "Sorry, you didn't give me enough gold.");
		return 1;
	}
	else {
		Item *item = new Item(bi);
		if (bi->getMaxStack())
			item->setQty(amount);
		//Armors between level 41 and 97 given this way become shoddy
		if (item->getType() == BaseItem::EQUIP) {
			unsigned short level = ((Equipment*)item)->levelReq();
			if (level < 99 && level > 40 && ((Equipment*)item)->getSlot() == Equipment::ARMOR)
				((Equipment*)item)->setMod(Equipment::SHODDY);
		}
		if (!c->getItem(item)) {
			lua_pushstring(L, "You are carrying too much.");
			delete item;
			return 1;
		}
		c->addGold(-(bi->getVal()*amount));
		return 0;
	}
}

int script::getSkillLevel(lua_State *L)
{
	if (lua_gettop(L) < 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	int id = lua_tonumber(L, 2);

	lua_pushnumber(L, c->skillLevel(id));
	return 1;
}

int script::repair(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int slot = (unsigned int)lua_tonumber(L, 2);
	if (slot >= NUM_ITEMS)
		return 0;

	lua_pushboolean(L, c->repair(slot));
	return 1;
}

int script::getSaleValue(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int slot = (unsigned int)(lua_tonumber(L, 2));
	unsigned int amt;
	if (n == 3) {
		amt = (unsigned int)(lua_tonumber(L, 3));
	}
	else {
		amt = 1;
	}
	if (slot > NUM_ITEMS) {
		//TODO a player tried to sell an item in a non-existent slot
		return 0;
	}

	Item *i = c->getInventory()[slot];
	if(i) {
		int val = i->getValue() / 2;
		if (i->getMaxDur())
			val = val * i->getDur() / i->getMaxDur();
		val *= amt;
		lua_pushnumber(L, (double)val);
		return 1;
	}

	//There was nothing in the slot
	return 0;
}

int script::buyItem(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int slot = (unsigned int)(lua_tonumber(L, 2));
	unsigned int amt;
	if (n == 3)
		amt = (unsigned int)(lua_tonumber(L, 3));
	else
		amt = 1;

	if (slot > NUM_ITEMS) {
		//TODO player tried to sell an item in a non-existent slot
		return 0;
	}

	Item *i = c->getInventory()[slot];
	if (i) {
		int val = i->getValue() / 2;
		if (i->getMaxDur())
			val = val * i->getDur() / i->getMaxDur();

		Item *d = c->removeItem(slot, amt);
		assert(d);
		if (d) {//TODO error logging
			if (d->getMaxQty())
				val *= d->getQty();
			delete d;
			c->addGold(val);
		}
	}
	return 0;
}

int script::getSkillSublist(lua_State *L)
{
	//Get a list of skills which the player has the right class to learn
	const int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);

	lua_newtable(L);
	for (int i = 2, slot = 1; i <= n; i++) {
		int id = (int)lua_tonumber(L, i);
		SkillInfo *sk = SkillInfo::getById(id);
		if (sk) {
			//Path qualifies?
			if (sk->path && !((c->getPathMask() & sk->path) > 0)) {
				continue;
			}
			//Doesn't have already?
			if (c->hasSkill(id)) {
				continue;
			}
			lua_pushnumber(L, slot++);
			lua_pushnumber(L, id);
			lua_settable(L, -3);
		}
	}

	return 1;
}

int script::setHairColor(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned char color = (unsigned char)lua_tonumber(L, 2);

	c->setHaircolor(color);
	return 0;
}

int script::giveSkill(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 3)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	int id = (int)lua_tonumber(L, 2);
	bool isSpell = lua_toboolean(L, 3);

	bool isFree = false;
	if (n >= 4 && lua_toboolean(L, 4))
		isFree = true;

	Secret *sp;
	Skill *sk;
	if (isSpell) {
		sp = new Secret(id, c->getPath());
		sk = sp;
	}
	else {
		sk = new Skill(id, c->getPath());
	}

	if (!sk) {
		//TODO log error
		lua_pushstring(L, "Sorry, I can't teach you that right now.");
		return 1;
	}

	//Check requirements

	//Path requirement
	if (sk->getBase()->path && !(c->getPathMask() & sk->getBase()->path)) {
		lua_pushstring(L, "This skill is forbidden to your path.");
		return 1;
	}
	//Unlearned requirement
	if (isSpell && c->hasSecret(id)) {
		lua_pushstring(L, "You already know this secret.");
		return 1;
	}
	else if (!isSpell && c->hasSkill(id)) {
		lua_pushstring(L, "You already know this skill.");
		return 1;
	}

	// If a skill is being given for free, only the basic
	// requirements above must be met
	if (!isFree) {
		//Stat requirements
		if (!sk->canLearn(c->getStats(), c->getLevel())) {
			char buffer[200];
			snprintf(buffer, 200, "You must have the following attributes before you may learn it:"
					" Insight %hd STR %hd DEX %hd CON %hd INT %hd WIS %hd",
					sk->getBase()->level,
					sk->getBase()->str,
					sk->getBase()->dex,
					sk->getBase()->con,
					sk->getBase()->int_,
					sk->getBase()->wis);
			lua_pushstring(L, buffer);
			delete sk;
			return 1;
		}

		//Prerequisite skill requirements
		for (auto it = sk->getBase()->reqs.begin(); it != sk->getBase()->reqs.end(); it++) {
			if (c->skillLevel(it->id) < it->lev) {
				SkillInfo *si = SkillInfo::getById(it->id);
				if (si) {
					char buffer[200];
					snprintf(buffer, 200, "You must improve %s to %hd before you may learn this.",
							si->name,
							it->lev);
					lua_pushstring(L, buffer);
					return 1;
				}
				else {
					//TODO log error
					lua_pushstring(L, "You do not know all of the prerequisite skills.");
					return 1;
				}
			}
		}

		//Charge for the skill - check cost requirements first
		if (((int)c->getGold()) < sk->getBase()->gold) {
			char buffer[100];
			snprintf(buffer, 100, "It costs %d gold to learn this skill.", sk->getBase()->gold);
			delete sk;
			lua_pushstring(L, buffer);
			return 1;
		}
		//unsigned short slot1, slot2, slot3;
		if (sk->getBase()->item1 && c->countItems(sk->getBase()->item1, sk->getBase()->item1mod) < sk->getBase()->item1Qty) {
			lua_pushstring(L, "Sorry, you don't have all of the required items.");
			delete sk;
			return 1;
		}
		if (sk->getBase()->item2 && c->countItems(sk->getBase()->item2, sk->getBase()->item2mod) < sk->getBase()->item2Qty) {
			lua_pushstring(L, "Sorry, you don't have all of the required items.");
			delete sk;
			return 1;
		}
		if (sk->getBase()->item3 && c->countItems(sk->getBase()->item3, sk->getBase()->item3mod) < sk->getBase()->item3Qty) {
			lua_pushstring(L, "Sorry, you don't have all of the required items.");
			delete sk;
			return 1;
		}
	}

	char slot;
	if (isSpell)
		slot = c->learnSecret(sp);
	else
		slot = c->learnSkill(sk);
	if (!slot) {
		lua_pushstring(L, "You cannot learn any more.");
		delete sk;
		return 1;
	}

	if (!isFree) {
		c->addGold(- (sk->getBase()->gold));
		Item *d;
		if (sk->getBase()->item1) {
			//c->removeItem(slot1, sk->getBase()->item1Qty);
			d = c->getInventory().take(sk->getBase()->item1, sk->getBase()->item1Qty, sk->getBase()->item1mod, c);
			assert(d);
			if (d)
				delete d;
		}
		if (sk->getBase()->item2) {
			d = c->getInventory().take(sk->getBase()->item2, sk->getBase()->item2Qty, sk->getBase()->item2mod, c);
			assert(d);
			if (d)
				delete d;
		}
		if (sk->getBase()->item3) {
			d = c->getInventory().take(sk->getBase()->item3, sk->getBase()->item3Qty, sk->getBase()->item3mod, c);
			assert(d);
			if (d)
				delete d;
		}
	}

	return 0;
}

int script::canSellItem(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int slot = (unsigned int)lua_tonumber(L, 2);

	bool r = false;
	Item *item;
	if (slot < NUM_ITEMS && (item = c->getInventory()[slot])) {
		for (int i = 3; i <= n; i++) {
			if (item->getId() == (int)lua_tonumber(L, i)) {
				r = true;
				break;
			}
		}
	}

	lua_pushboolean(L, r);
	return 1;
}

int script::setHairstyle(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 2)
		return 0;
	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned char style = (unsigned char)lua_tonumber(L, 2);
	c->setHairstyle(style);
	return 0;
}
int script::withdrawItem(lua_State *L)
{
	if (lua_gettop(L) < 3) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int slot = lua_tonumber(L, 2);
	int qty = lua_tonumber(L, 3);
	const char *itemName = lua_tostring(L, 2);

	int amt;
	if (lua_gettop(L) > 2)
		amt = (int)lua_tonumber(L, 3);
	else
		amt = 1;

	const std::vector<Character::StorageItem> &storage = c->getStorage();
	for (int i = 0; i < storage.size(); i++) {
		//Check each lame
		BaseItem *bi = BaseItem::getById(storage[i].id);
		Item tmp(bi);
		if (storage[i].mod)
			((Equipment *)(&tmp))->setMod(storage[i].mod);
		if (!strcmp(itemName, tmp.getName())) {
			c->withdraw(i, amt);
			return 0;
		}
	}
	lua_pushboolean(L, c->withdraw(slot, qty));

	return 1;
}
int script::depositGold(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int amt = lua_tonumber(L, 2);

	lua_pushboolean(L, c->depositGold(amt));
	return 1;
}
int script::withdrawGold(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	unsigned int amt = lua_tonumber(L, 2);

	lua_pushboolean(L, c->withdrawGold(amt));
	return 1;
}
int script::countStoredGold(lua_State *L)
{
	if (lua_gettop(L) < 1) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	lua_pushnumber(L, c->getStoredGold());
	return 1;
}
int script::storageInfo(lua_State *L)
{
	if (lua_gettop(L) < 2)
		return 0;

	Character *c = (Character *)lua_touserdata(L, 1);
	const char *itemName = lua_tostring(L, 2);

	const std::vector<Character::StorageItem> &storage = c->getStorage();
	char buffer[100];
	for (int i = 0; i < storage.size(); i++) {
		BaseItem *bi = BaseItem::getById(storage[i].id);
		if (!bi) {
			LOG4CPLUS_ERROR(script::log, "Character " << c->getName() << " is storing item with ID " << storage[i].id
					<< ", which doesn't identify an item!\n");
			continue;
		}

		if (storage[i].mod) {
			snprintf(buffer, 100, "%s %s",
					Equipment::getModName(((BaseEquipment*)bi)->getSlot(), storage[i].mod),
					bi->getName());
			if (!strcmp(buffer, itemName)) {
				lua_pushnumber(L, i);
				lua_pushnumber(L, bi->getMaxStack() ? storage[i].qty : 0);
				return 2;
			}
		}
		else {
			if (!strcmp(bi->getName(), itemName)) {
				lua_pushnumber(L, i);
				lua_pushnumber(L, bi->getMaxStack() ? storage[i].qty : 0);
				return 2;
			}
		}
	}

	return 0;
}
int script::canDeposit(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int slot = (int)lua_tonumber(L, 2);

	if (slot < 0 || slot >= NUM_ITEMS)
		return 0;
	lua_pushboolean(L, (c->getInventory()[slot])->canDeposit());
	return 1;
}
int script::chargeGold(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int amt = (int)lua_tonumber(L, 2);

	if ((signed)c->getGold() >= amt) {
		c->addGold(-amt);
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}
int script::depositItem(lua_State *L)
{
	if (lua_gettop(L) < 3) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int slot = (int)lua_tonumber(L, 2);
	int amt = (int)lua_tonumber(L, 3);

	Item *itm = c->removeItem(slot, amt);
	if (itm) {
		c->deposit(itm);
		delete itm;
	}
	return 0;
}

int script::getItemId(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return 0;
	}

	Character *c = (Character *)lua_touserdata(L, 1);
	int slot = lua_tonumber(L, 2);

	if (slot > NUM_ITEMS || slot < 0 || !c->getInventory()[slot])
		return 0;

	lua_pushnumber(L, c->getInventory()[slot]->getId());
	return 1;
}

/**
 * Returns true iff the character is in a guild.
 *
 */
int script::inGuild(lua_State *L)
{
	Character *c = luaGetCharacter(L);

	if (!c) {
		return luaL_error(L, "Invalid Character reference given in call to script::inGuild.");
	}

	lua_pushboolean(L, c->getGuild() != 0);
	return 1;
}

/**
 * Creates a guild and adds the specified character to it.
 * \param[in] L A lua state. The first stack parameter should be a character
 * reference. The second parameter should be a string with the name of the
 * guild to be created. Returns nil on success, or an error string on failure.
 * \return The number of return values pushed onto the lua stack.
 */
int script::createGuild(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "At least two arguments are required in "
				"script::createGuild.");
	}

	Character *c = luaGetCharacter(L);
	const char *str = luaL_checkstring(L, 2);

	if (!c) {
		return luaL_error(L, "Invalid Character reference given in call to "
				"script::createGuild.");
	}

	if (c->getGuild()) {
		lua_pushstring(L, "You are already in a guild.");
		return 1;
	}

	Guild *g = DataService::getService()->createGuild(str);
	//TODO add error messages down there
	if (!DataService::getService()->addGuildMember(g, c, Guild::LEADER)) {
		lua_pushstring(L, "Something went wrong.");
		return 1;
	}

	return 0;
}

int script::listMembers(lua_State *L)
{
	Character *c = luaGetCharacter(L);
	if (!c) {
		return luaL_error(L, "Invalid Character reference given in call to "
				"script::listMembers.");
	}

	// In a guild?
	Guild *g = c->getGuild();
	if (!g) {
		return 0;
	}

	char buffer[1024];
	char *tbuff = buffer;
	int len = 1024, amt;

	for (std::pair<std::string, Guild::Rank> memberInfo : g->getMemberList()){
		amt = snprintf(tbuff, len, "%s %s\n", memberInfo.first.c_str(),
				       g->getRankName(memberInfo.second));
		if (amt > len) {
			break;
		}
		len -= amt;
		tbuff += amt;
	}
	Server::sendMessage(c->getSession(), buffer, 8);

	return 0;
}

/**
 * \brief Add a player to a character's guild.
 *
 * Add a character to another character's guild. To succeed, the the first
 * character must be in a guild, and the second must not be in a guild.
 * \param[in] L A lua state. The first stack parameter should be the character
 * already in a guild. The second stack parameter should be the OID of the
 * character to be added to a guild. nil will be returned on success. On
 * failure, an error message will be returned.
 * \return The number of return values pushed onto the lua stack.
 */
int script::addMember(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "At least two arguments required in "
				"script::addMember.");
	}

	Character *c = luaGetCharacter(L);
	int oid = luaL_checknumber(L, 2);
	Character *n = (Character *)Entity::getByOid(oid);

	if (!c || !n) {
		return luaL_error(L, "Invalid Character reference given in call to "
				"script::addMember.");
	}

	// Does c have permission to add someone to a guild?
	if (c->getRank() != Guild::COUNCIL && c->getRank() != Guild::LEADER) {
		lua_pushstring(L, "You do not have permission to add members to the "
				"guild.");
		return 1;
	}
	// Is n not in a guild?
	if (n->getGuild()) {
		lua_pushstring(L, "They are already in a guild.");
		return 1;
	}

	// Add n to c's guild
	if (!DataService::getService()->addGuildMember(c->getGuild(), n)) {
		lua_pushstring(L, "Something went wrong.");
		return 1;
	}

	return 0;
}

/**
 * Disband the current guild. All members will be removed.
 * \param[in] L A lua state. The first parameter should be a character
 * reference. The character's guild will be disbanded, provided that character
 * is the guild leader. Returns nil on success, or an error message on
 * failure.
 * \return The number of return values pushed onto the lua stack.
 */
int script::deleteGuild(lua_State *L)
{
	Character *c = luaGetCharacter(L);

	if (!c) {
		luaL_error(L, "Invalid Character reference given in call to "
				"script::deleteGuild.");
	}

	Guild *g = c->getGuild();
	if (!g) {
		lua_pushstring(L, "You are not in a guild.");
		return 1;
	}

	if (c->getRank() != Guild::LEADER) {
		lua_pushstring(L, "You are not the guild leader.");
		return 1;
	}

	DataService::getService()->deleteGuild(g);
	return 0;
}

/**
 * Promote a character to council. Requires the name of the character to be
 * removed.
 * \param[in] L A lua state. The first parameter must be a character reference
 * and the second parameter must be a name. Returns a message if something
 * went wrong, and nil if the removal was successful.
 * \return The number of return values pushed onto the lua stack.
 */
int script::promoteMember(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "At least two arguments are required in "
				"script::promoteMember.");
	}

	Character *c = luaGetCharacter(L);
	const char *str = luaL_checkstring(L, 2);

	if (!c) {
		return luaL_error(L, "Invalid Character reference given in call to "
				"script::promoteMember.");
	}

	// Can c promote the selected person?
	if (!c->getGuild()) {
		lua_pushstring(L, "You are not in a guild.");
		return 1;
	}
	Guild::Rank trank = c->getGuild()->getRank(str);
	// Not a member?
	if (trank == Guild::NON_MEMBER) {
		lua_pushstring(L, "They don't seem to be a member of your guild.");
		return 1;
	}
	// No one can "promote" the leader
	if (trank == Guild::LEADER) {
		lua_pushstring(L, "The guild leader cannot be promoted to council.");
		return 1;
	}
	// Only works if the promoter is the leader, and the target is a member
	if (c->getRank() == Guild::LEADER && trank == Guild::MEMBER) {
		DataService::getService()->promoteGuildMember(c->getGuild(), str, Guild::COUNCIL);
		return 0;
	}

	lua_pushstring(L, "You can't promote this member.");
	return 1;
}

/**
 * Remove a character from a guild. Requires the name of the character to be
 * removed.
 * \param[in] L A lua state. The first parameter must be a character reference
 * and the second parameter must be a name. Returns a message if something
 * went wrong, and nil if the removal was successful.
 * \return The number of return values pushed onto the lua stack.
 */
int script::removeMember(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "At least two arguments are required in "
				"script::removeMember.");
	}

	Character *c = luaGetCharacter(L);
	const char *str = luaL_checkstring(L, 2);

	if (!c) {
		return luaL_error(L, "Invalid Character reference given in call to "
				"script::removeMember.");
	}

	// Can c remove the selected person?
	if (!c->getGuild()) {
		lua_pushstring(L, "You are not in a guild.");
		return 1;
	}
	Guild::Rank trank = c->getGuild()->getRank(str);
	// Not a member?
	if (trank == Guild::NON_MEMBER) {
		lua_pushstring(L, "They don't seem to be a member of your guild.");
		return 1;
	}
	// No one can remove the leader
	if (trank == Guild::LEADER) {
		lua_pushstring(L, "The guild leader can never be removed.");
		return 1;
	}
	// Beyond that, anyone can remove themselves
	bool canRemove = false;
	std::string cname(c->getName());
	std::string tname(str);
	lower(cname);
	lower(tname);
	// and additionally, the leader can remove anyone and council can remove
	// normal members
	canRemove = cname == tname || c->getRank() == Guild::LEADER ||
			(c->getRank() == Guild::COUNCIL && trank == Guild::MEMBER);
	if (canRemove) {
		DataService::getService()->removeGuildMember(c->getGuild(), str);
		return 0;
	}

	lua_pushstring(L, "You don't have permission to remove this member.");
	return 1;
}
