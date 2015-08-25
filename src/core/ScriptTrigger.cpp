/*
 * ScriptTrigger.cpp
 *
 *  Created on: 2013-04-10
 *      Author: per
 */

#include "ScriptTrigger.h"
#include "npc_bindings.h"
#include "Character.h"

std::atomic_int ScriptTrigger::ids(0);

ScriptTrigger::ScriptTrigger(short x, short y, Map *map, int lifetime) :
    Trigger(x, y, map), lifetime(lifetime * TICKS), triggerId(++ids)
{
}

ScriptTrigger::~ScriptTrigger()
{
}

bool ScriptTrigger::tick()
{
    //script triggers which are effects of other triggers do expire
    if (lifetime > 0)
        return --lifetime == 0;
    return false;
}

bool ScriptTrigger::operator()(Entity *e)
{
    if (e->getType() != Entity::E_CHARACTER)
        return false;
    return initScript(((Character *) e), this);
}

bool ScriptTrigger::operator()(Item *itm, Entity *dropper)
{
    if (dropper->getType() != Entity::E_CHARACTER)
        return false;

    return initScript(((Character *) dropper), this, itm);
}
