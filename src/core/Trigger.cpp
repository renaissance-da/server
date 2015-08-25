/*
 * Trigger.cpp
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#include "Trigger.h"
#include "core.h"
#include "log4cplus/loggingmacros.h"

Trigger::Trigger(unsigned short x, unsigned short y, Map *map) :
    Entity(x, y, map, "Trigger")
{
}

Trigger::~Trigger()
{
}

void Trigger::getViewedBlock(IDataStream *d)
{
    LOG4CPLUS_WARN(core::log, "getViewedBlock called on a trigger object.");
}

short Trigger::getHideLevel()
{
    return 99;
}
