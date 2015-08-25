/*
 * Trigger.h
 *
 *  Created on: 2013-03-02
 *      Author: per
 */

#ifndef TRIGGER_H_
#define TRIGGER_H_

#include "MapPoint.h"
#include "Item.h"
#include "Entity.h"

class Trigger: public Entity
{
public:
    Trigger(unsigned short x, unsigned short y, Map *map);
    virtual ~Trigger();

    virtual bool tick()
    {
        return false;
    }
    virtual bool operator()(Entity *e) = 0;
    virtual bool operator()(Item *itm, Entity *dropper)
    {
        return false;
    }

    void getViewedBlock(IDataStream *d) override;
    short getHideLevel() override;
};

#endif /* TRIGGER_H_ */
