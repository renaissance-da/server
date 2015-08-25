/*
 * Item.h
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#ifndef ITEM_H_
#define ITEM_H_
#include "BaseItem.h"
#include "BaseEquipment.h"
#include "BaseConsumable.h"
#include "IDataStream.h"
#include <string>
class Character;

class Item
{
public:

    enum DeathAction
    {
        PERISH, DROP, BOUND
    };

    Item(BaseItem *base);
    Item(unsigned int id, unsigned short qty, unsigned int dur);
    ~Item();

    BaseItem::ItemType getType()
    {
        return base->getType();
    }
    unsigned short getWeight()
    {
        return weight;
    }
    unsigned short getApr()
    {
        return base->getApr();
    }
    void getTradeView(IDataStream *dp, char pos);
    unsigned int getDur()
    {
        return dur;
    }
    unsigned int getMaxDur()
    {
        return maxDur;
    }
    unsigned char getQty()
    {
        return base->getMaxStack() ? qty : 0;
    }
    unsigned char getMaxQty()
    {
        return base->getMaxStack();
    }
    const char *getName();
    unsigned short getNameLen();
    int getId()
    {
        return base->getId();
    }
    void addQty(short qty)
    {
        this->qty += qty;
    }
    void setMaxedQty()
    {
        qty = base->getMaxStack();
    }
    void setQty(unsigned short qty)
    {
        this->qty = qty;
    }
    unsigned int getValue()
    {
        return base->getVal();
    }
    int getMod()
    {
        return mod;
    }
    bool canModify()
    {
        return base->modify();
    }
    bool isBound()
    {
        return base->getBaseFlags() & 1;
    }
    bool canDeposit();
    DeathAction getDeathAction();
    unsigned int repairCost();
    void repair();
    bool canRepair();
    bool isIdentified()
    { /*TODO implement*/
        return false;
    }

protected:
    union
    {
        BaseItem *base;
        BaseEquipment *baseEq;
        BaseConsumable *baseCon;
    };

    //These end up being type dependent, maybe the subclasses should be virtual?
    unsigned short qty;
    unsigned int dur, maxDur;
    const Stats *stats;
    int mod;
    unsigned short weight;
    std::string name;
};

#endif /* ITEM_H_ */
