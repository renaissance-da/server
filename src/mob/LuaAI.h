/*
 * LuaAI.h
 *
 *  Created on: Feb 15, 2014
 *      Author: per
 */

#ifndef LUAAI_H_
#define LUAAI_H_

#include <MobAI.h>
#include <lua5.2/lua.hpp>

class LuaAI: public MobAI {
public:
	LuaAI(int submode, Mob *m);
	virtual ~LuaAI();

	void runAI();
private:
	int useSkill(lua_State *L);
	int move(lua_State *L);
	int turn(lua_State *L);
	int talk(lua_State *L);

	lua_State *L;
};

#endif /* LUAAI_H_ */
