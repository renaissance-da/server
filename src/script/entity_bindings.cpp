/*
 * entity_bindings.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */


#include "entity_bindings.h"
#include "Map.h"
#include "script.h"
#include "Combat.h"

Entity *script::getEntity(lua_State *L, int narg)
{
	assert(lua_gettop(L) >= narg && narg > 0);
	luaL_checktype(L, narg, LUA_TLIGHTUSERDATA);
	return (Entity *)lua_touserdata(L, narg);
}

void script::loadEntityBindings(lua_State *L)
{
	REGISTER(talk);
	REGISTER(getName);
	REGISTER(getLevel);
	REGISTER(getPosition);
	REGISTER(restoreMp);
	REGISTER(getId);
	REGISTER(drainHp);
	REGISTER(playSound);
	REGISTER(playEffect);
	REGISTER(turn);
	REGISTER(move);
	REGISTER(useSkill);
	REGISTER(getType);
	REGISTER(removeEffect);
}

int script::talk(lua_State *L)
{
	if (lua_gettop(L) < 2) {
		return luaL_error(L, "Insufficient arguments given in call to %s.", __PRETTY_FUNCTION__);
	}

	Entity *e = (Entity *)lua_touserdata(L, 1);
	const char *text = lua_tostring(L, 2);

	Map *m = e->getMap();
	m->talked(e, text);
	return 0;
}

int script::getName(lua_State *L)
{
	if (lua_gettop(L) < 1)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	lua_pushstring(L, e->getName().c_str());
	return 1;
}

int script::getPosition(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	lua_pushnumber(L, (double)e->getMapId());
	lua_pushnumber(L, (double)e->getX());
	lua_pushnumber(L, (double)e->getY());

	return 3;
}

int script::getLevel(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	lua_pushnumber(L, (double)(e->getLevel()));

	return 1;
}

int script::restoreMp(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;
	Entity *e = (Entity *)lua_touserdata(L, 1);
	float amt = (float)lua_tonumber(L, 2);
	e->addMp(amt);
	return 0;
}

int script::getId(lua_State *L)
{
	int n = lua_gettop(L);

	if (n < 1) {
		return 0;
	}

	assert(lua_type(L, 1) == LUA_TLIGHTUSERDATA);
	Entity *e = (Entity *)lua_touserdata(L, 1);

	if (e)
	    lua_pushnumber(L, (double)e->getOid());
	else
	    lua_pushnumber(L, 0);
	return 1;
}

int script::drainHp(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;
	Entity *e = (Entity *)lua_touserdata(L, 1);
	int dmg = (int)lua_tonumber(L, 2);
	e->damage(0, dmg);
	return 0;
}

int script::playSound(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	char sound = (char)lua_tonumber(L, 2);

	e->makeSound(sound);

	return 0;
}

int script::playEffect(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	unsigned short effect = (unsigned short)lua_tonumber(L, 2);

	e->playEffect(effect, 0x64);
	return 0;
}

int script::move(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	char dir = (char)lua_tonumber(L, 2);
	lua_pushboolean(L, e->tryMove(dir));
	return 1;
}

int script::turn(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Entity *e = (Entity *)lua_touserdata(L, 1);
	char dir = (char)lua_tonumber(L, 2);
	lua_pushboolean(L, e->tryTurn(dir));
	return 1;
}

int script::useSkill(lua_State *L)
{
	//User, target, skill id
	if (lua_gettop(L) < 2)
		return 0;

	Entity *atk = (Entity *)lua_touserdata(L, 1);
	int skillId = (int)lua_tonumber(L, 2);
	Entity *tgt = 0;
	if (lua_gettop(L) >= 3)
		tgt = (Entity *)lua_touserdata(L, 3);

	Skill *sk = new Skill(skillId, Path::Peasant, 100);
	Combat::useSkill(atk, sk, tgt);
	delete sk;

	return 0;
}

int script::getType(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return 0;

	Entity *e = getEntity(L, 1);
	lua_pushnumber(L, (int)e->getType());

	return 1;
}

int script::removeEffect(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return 0;

	Entity *e = getEntity(L, 1);
	int effect = luaL_checkinteger(L, 2);
	if (effect >= StatusEffect::SE_KINDS) {
		luaL_error(L, "%s called on invalid kind %d", __PRETTY_FUNCTION__, effect);
	}

	lua_pushboolean(L, e->removeEffect((StatusEffect::Kind)effect));
	return 1;
}
