/*
 * entity_bindings.h
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */

#ifndef ENTITY_BINDINGS_H_
#define ENTITY_BINDINGS_H_

#include "lua.hpp"
#include "Entity.h"

namespace script
{

void loadEntityBindings(lua_State *L);
Entity *getEntity(lua_State *L, int narg);

int talk(lua_State *L);
int getName(lua_State *L);
int getPosition(lua_State *L);
int getLevel(lua_State *L);
int restoreMp(lua_State *L);
int getId(lua_State *L);
int drainHp(lua_State *L);
int playSound(lua_State *L);
int playEffect(lua_State *L);
int move(lua_State *L);
int turn(lua_State *L);
int useSkill(lua_State *L);
int getType(lua_State *L);
int removeEffect(lua_State *L);

}

#endif /* ENTITY_BINDINGS_H_ */
