/*
 * Item.cpp
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#include "Item.h"
#include "Character.h"
#include "srv_proto.h"
#include "defines.h"
#include <assert.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

Item::Item(BaseItem *base) :
    base(base), stats(0), mod(0)
{
    if (base->getMaxStack())
        qty = 1;
    else
        qty = 0;
    dur = base->getMaxDur();
    weight = base->getWeight();
    maxDur = base->getMaxDur();
    name = base->getName();
}

Item::Item(unsigned int id, unsigned short qty, unsigned int dur) :
    qty(qty), dur(dur), stats(0), mod(0)
{
    base = BaseItem::getById(id);
    assert(base);
    weight = base->getWeight();
    maxDur = base->getMaxDur();
    name = base->getName();
}

unsigned short Item::getNameLen()
{
    return name.length();
}

const char *Item::getName()
{
    return name.c_str();
}

Item::~Item()
{
    if (stats)
        delete stats;
}

unsigned int Item::repairCost()
{
    if (base->getType() == BaseItem::EQUIP) {
        return ((base->getVal() >> 1) * (maxDur - dur) / maxDur);
    }
    return 0;
}

bool Item::canRepair()
{
    return base->getType() == BaseItem::EQUIP && maxDur > dur;
}

void Item::repair()
{
    dur = maxDur;
}

void Item::getTradeView(IDataStream *dp, char pos)
{
    dp->appendByte(pos);
    dp->appendShort(getApr());
    dp->appendByte(0);
    char buffer[128];
    unsigned char len;
    if (getMaxQty())
        len = snprintf(buffer, 128, "%s %d", getName(), getQty());
    else if (maxDur)
        len = snprintf(buffer, 128, "%s %d%%", getName(), (dur * 100) / maxDur);
    else
        len = snprintf(buffer, 128, "%s", getName());
    dp->appendString(len, buffer);
}

Item::DeathAction Item::getDeathAction()
{
    if (base->getBaseFlags() & 2)
        return PERISH; //Potentially an item could be both bound and perishable
    if (base->getBaseFlags() & 5) //Bound or just keep on death
        return BOUND;
    return mod > 0 ? PERISH : DROP;
}

bool Item::canDeposit()
{
    //TODO check for identified, other restrictions?
    return (dur == maxDur);
}
