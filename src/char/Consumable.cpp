/*
 * Consumable.cpp
 *
 *  Created on: 2013-01-07
 *      Author: per
 */

#include "Consumable.h"
#include "skills.h"
#include "Combat.h"
#include "Entity.h"

Consumable::Consumable(unsigned int id, unsigned short qty, unsigned int dur) :
    Item(id, qty, dur)
{
}

Consumable::~Consumable()
{
}
