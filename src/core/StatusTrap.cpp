/*
 * StatusTrap.cpp
 *
 *  Created on: 2013-05-28
 *      Author: per
 */

#include "StatusTrap.h"
#include "defines.h"
#include "DataService.h"

StatusTrap::StatusTrap(short x, short y, Map *map, int seid,
		       Entity::EntityType user) :
    Trigger(x, y, map), statusId(seid), userType(user), lifetime(120 * TICKS)
{
}

bool StatusTrap::operator()(Entity *e)
{
    Map *m = e->getMap();

    if (userType == e->getType()
        && (userType != Entity::E_CHARACTER || !m->pvpOn())) {
        return false;
    }

    e->makeSound(1);
    e->addStatusEffect(statusId);
    return true;
}

bool StatusTrap::tick()
{
    return --lifetime <= 0;
}
