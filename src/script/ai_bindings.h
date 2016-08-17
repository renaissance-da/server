/*
 * ai_bindings.h
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */

#ifndef AI_BINDINGS_H_
#define AI_BINDINGS_H_

#include "lua.hpp"
#include "Mob.h"

namespace script
{

Mob *getMob(lua_State *L, int narg, const char *caller);

void loadAIBindings(lua_State *L);

int useSingle(lua_State *L);
int useAoe(lua_State *L);
int getMinions(lua_State *L);
int spawnMob(lua_State *L);

void act(lua_State *L, std::vector<Entity *> &nearby, std::vector<Entity *> &targets);
lua_State *newLuaAI(int submode, Mob *m);
void freeLuaAI(lua_State *L);

} // namespace script



#endif /* AI_BINDINGS_H_ */
