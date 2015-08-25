/*
 * map_bindings.cpp
 *
 *  Created on: May 2, 2014
 *      Author: per
 */


#include "map_bindings.h"
#include "Entity.h"
#include "Map.h"
#include "Mob.h"
#include "MobInfo.h"
#include "ScriptTimer.h"
#include "DataService.h"

int script::isWall(lua_State *L)
{
	Entity *e = (Entity *)lua_touserdata(L, 1);
	unsigned short x = (unsigned short)lua_tonumber(L, 2);
	unsigned short y = (unsigned short)lua_tonumber(L, 3);

	lua_pushboolean(L, e->getMap()->isWall(x, y, e->moveType()));
	return 1;
}

void script::loadMapBindings(lua_State *L)
{
	REGISTER(isWall);
	REGISTER(spawnNewMob);
	REGISTER(addTimer);
}

int script::addTimer(lua_State *L)
{
	const int n = lua_gettop(L);
	if (n < 3) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	int mapid = lua_tonumber(L, 1);
	int duration = lua_tonumber(L, 2);
	int timerId = lua_tonumber(L, 3);

	Map *m = DataService::getService()->getMap(mapid);
	if (!m) {
		return luaL_error(L, "Invalid map number (%d) given in call to %s.", mapid, __PRETTY_FUNCTION__);
	}

	ScriptTimer *st = new ScriptTimer(timerId, time(NULL) + duration);
	//TODO can we lock this?
	m->addTimer(st);

	return 0;
}

int script::spawnNewMob(lua_State *L)
{
	if (lua_gettop(L) < 4)
		return 0;

	int mapid = lua_tonumber(L, 1);
	int id = lua_tonumber(L, 2);
	short x = lua_tonumber(L, 3);
	short y = lua_tonumber(L, 4);

	Map *map = DataService::getService()->getMap(mapid);
	if (!map) {
		return luaL_error(L, "Invalid map number given in call to %s.", __PRETTY_FUNCTION__);
	}
	Mob *mob = mob::MobInfo::spawnById(id, x, y, map);
	if (!mob) {
	    return luaL_error(L, "Failed to spawn mob using ID=%d", id);
	}	

	return 0;
}
