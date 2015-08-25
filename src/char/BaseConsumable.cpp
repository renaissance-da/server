/*
 * BaseConsumable.cpp
 *
 *  Created on: 2013-01-07
 *      Author: per
 */

#include "BaseConsumable.h"

BaseConsumable::BaseConsumable(const char *name, unsigned short groundApr,
    unsigned short wgt, unsigned int value, unsigned short maxStack,
    unsigned int id, short cType, int param, int flags) :
    BaseItem(name, groundApr, wgt, value, maxStack, id, false, CONSUME, flags), param(
        param)
{
    switch (cType) {
    case FOOD:
        this->cType = FOOD;
        break;
    case HP_POTION:
        this->cType = HP_POTION;
        break;
    case REVIVE:
        this->cType = REVIVE;
        break;
    case MP_POTION:
        this->cType = MP_POTION;
        break;
    case SCROLL:
        this->cType = SCROLL;
        break;
    case CURES:
        this->cType = CURES;
        break;
    case BUFFS:
        this->cType = BUFFS;
        break;
    default:
        this->cType = FOOD;
        break;
    }
}

BaseConsumable::~BaseConsumable()
{
}

