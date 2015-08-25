/*
 * Trap.cpp
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#include "Trap.h"
#include "Character.h"
#include "DataService.h"
#include "srv_proto.h"

Trap::Trap(short x, short y, Map *map, Entity *user, int dmg) :
    Trigger(x, y, map), lifetime(120 * TICKS), dmg(dmg)
{
    userType = user->getType();
    uoid = user->getOid();
}

bool Trap::operator()(Entity *e)
{
    Map *m = e->getMap();
    if (userType == Entity::E_CHARACTER) {
        if (!m->pvpOn() && e->getType() == Entity::E_CHARACTER)
            return false;
    }
    else {
        if (userType == e->getType())
            return false;
    }
    //Trap affects the target
    Entity *user = Entity::getByOid(uoid);
    if (user && user->dist(e) > NEARBY)
        user = 0;
    e->struck(user, dmg);

    return true;
}

bool Trap::tick()
{
    return --lifetime <= 0;
}
