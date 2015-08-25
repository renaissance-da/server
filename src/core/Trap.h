/*
 * Trap.h
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#ifndef TRAP_H_
#define TRAP_H_

#include "Trigger.h"
#include "Entity.h"

class Trap: public Trigger
{
public:
    Trap(short x, short y, Map *map, Entity *user, int dmg);

    virtual bool operator()(Entity *e);bool tick();
private:
    Entity::EntityType userType;
    int uoid, lifetime;
    float dmg;
};

#endif /* TRAP_H_ */
