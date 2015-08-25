/*
 * ai_bindings.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */

#include "ai_bindings.h"
#include "script.h"
#include "log4cplus/loggingmacros.h"
#include "entity_bindings.h"
#include "map_bindings.h"
#include "Map.h"
#include "MobAI.h"
#include "DataService.h"
#include "MobInfo.h"

#define _getMob(L, n) getMob((L), (n), __PRETTY_FUNCTION__)

Mob *script::getMob(lua_State *L, int narg, const char *caller)
{
	Entity *e = getEntity(L, narg);
	Mob *m = dynamic_cast<Mob *>(e);
	if (!m)
		luaL_error(L, "Invalid call to script::getMob in %s. Given parameter is not a mob.", caller);
	return m;
}

void script::loadAIBindings(lua_State *L)
{
	REGISTER(useSingle);
	REGISTER(useAoe);
	REGISTER(getMinions);
	REGISTER(spawnMob);
}

int script::useSingle(lua_State *L)
{
	// Use a single target skill
	const int n = lua_gettop(L);
	if (n < 1)
		return luaL_error(L, "Expected at least 1 argument to %s.", __PRETTY_FUNCTION__);
	Mob *m = _getMob(L, 1);
	Entity *t = 0;

	if (n >= 2)
		t = getEntity(L, 2);

	lua_pushboolean(L, m->getAI()->useRandomSkill(t, 1.0, MobAI::SINGLE));
	return 1;
}

int script::getMinions(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 1)
		return luaL_error(L, "Expected 1 argument to %s.", __PRETTY_FUNCTION__);
	Mob *m = _getMob(L, 1);

	lua_newtable(L);
	int idx = 0;
	for (Mob *minion : m->getAI()->getMinions()) {
		lua_pushlightuserdata(L, minion);
		lua_rawseti(L, -2, ++idx);
	}

	return 1;
}

int script::spawnMob(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n != 5)
		return luaL_error(L, "Expected 5 arguments to %s.", __PRETTY_FUNCTION__);

	Mob *m = 0;
	if (!lua_isnil(L, 1))
		m = _getMob(L, 1);
	int mobid = luaL_checkinteger(L, 2);
	int mapid = luaL_checkinteger(L, 3);
	unsigned short x = luaL_checkinteger(L, 4);
	unsigned short y = luaL_checkinteger(L, 5);

	Map *map = DataService::getService()->getMap(mapid);
	if (!map) {
		return luaL_error(L, "Invalid map number given in call to %s.", __PRETTY_FUNCTION__);
	}
	Mob *minion = mob::MobInfo::spawnById(mobid, x, y, map);
	if (m) {
		m->getAI()->addMinion(minion);
		minion->spawnedBy(m->getAI());
	}

	lua_pushlightuserdata(L, minion);
	return 1;
}

int script::useAoe(lua_State *L)
{
	// Use a single target skill
	const int n = lua_gettop(L);
	if (n < 1)
		return luaL_error(L, "Expected at least 1 argument to %s.", __PRETTY_FUNCTION__);
	Mob *m = _getMob(L, 1);
	Entity *t = 0;

	if (n >= 2)
		t = getEntity(L, 2);

	lua_pushboolean(L, m->getAI()->useRandomSkill(t, 1.0, MobAI::AOE));
	return 1;
}

void script::act(lua_State *L, std::vector<Entity *> &nearby, std::vector<Entity *> &targets)
{
	//Push function name
	lua_getglobal(L, "act");

	//Populate input arrays with entity info
	lua_newtable(L);
	for (unsigned i = 0; i < nearby.size(); i++) {
		lua_pushlightuserdata(L, nearby[i]);
		lua_rawseti(L, -2, i+1);
	}
	lua_newtable(L);
	for (unsigned i = 0; i < targets.size(); i++) {
		lua_pushlightuserdata(L, targets[i]);
		lua_rawseti(L, -2, i+1);
	}

	if (lua_pcall(L, 2, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::act: "
			    << lua_tostring(L, -1));
	    lua_pop(L, 1);
	}
}

lua_State *script::newLuaAI(int submode, Mob *m)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	loadEntityBindings(L);
	loadAIBindings(L);
	loadMapBindings(L);

	int err = luaL_dofile(L, "script/mob.lua");

	if (err)
	    LOG4CPLUS_ERROR(script::log, "Error while loading mob.lua\n");

	lua_getglobal(L, "initMobAI");
	lua_pushnumber(L, (double)submode);
	lua_pushlightuserdata(L, (void *)m);

	if (lua_pcall(L, 2, 0, 0) != 0) {
	    LOG4CPLUS_ERROR(script::log, "Error in call to script::newLuaAI, with "
			    << "submode " << submode << ": " << lua_tostring(L, -1));
	    lua_pop(L, 1);
	}

	return L;
}

void script::freeLuaAI(lua_State *L)
{
	lua_close(L);
}
