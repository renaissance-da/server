/*
 * StatusTrap.h
 *
 *  Created on: 2013-05-28
 *      Author: per
 */

#ifndef STATUSTRAP_H_
#define STATUSTRAP_H_

#include "Trigger.h"
#include "Entity.h"

class StatusTrap: public Trigger
{
public:
    StatusTrap(short x, short y, Map *map, int seid,
	       Entity::EntityType user);

    virtual bool operator()(Entity *e);bool tick();

private:
    int statusId;
    Entity::EntityType userType;
    int lifetime;
};

#endif /* STATUSTRAP_H_ */
