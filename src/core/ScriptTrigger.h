/*
 * ScriptTrigger.h
 *
 *  Created on: 2013-04-10
 *      Author: per
 */

#ifndef SCRIPTTRIGGER_H_
#define SCRIPTTRIGGER_H_

#include "Trigger.h"
#include "Item.h"
#include <atomic>

class ScriptTrigger: public Trigger
{
public:
    ScriptTrigger(short x, short y, Map *map, int lifetime);
    virtual ~ScriptTrigger();

    bool operator()(Entity *e);
    bool operator()(Item *itm, Entity *dropper);
    bool tick();
    int getId()
    {
        return triggerId;
    }
private:

    int lifetime, triggerId;
    static std::atomic_int ids;
};

#endif /* SCRIPTTRIGGER_H_ */
