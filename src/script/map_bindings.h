/*
 * map_bindings.h
 *
 *  Created on: May 2, 2014
 *      Author: per
 */

#ifndef MAP_BINDINGS_H_
#define MAP_BINDINGS_H_

#include "lua.hpp"
#include "script.h"

namespace script
{

int isWall(lua_State *L);
int spawnNewMob(lua_State *L);
int addTimer(lua_State *L);

void loadMapBindings(lua_State *L);

}

#endif /* MAP_BINDINGS_H_ */
