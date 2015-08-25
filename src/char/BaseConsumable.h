/*
 * BaseConsumable.h
 *
 *  Created on: 2013-01-07
 *      Author: per
 */

#ifndef BASECONSUMABLE_H_
#define BASECONSUMABLE_H_

#include "BaseItem.h"

class BaseConsumable: public BaseItem
{
public:
    enum ConsumableType
    {
        FOOD = 0,
        HP_POTION = 1,
        REVIVE = 2,
        MP_POTION = 3,
        SCROLL = 4,
        CURES = 5,
        BUFFS = 6
    };

    BaseConsumable(const char *name, unsigned short groundApr,
        unsigned short wgt, unsigned int value, unsigned short maxStack,
        unsigned int id, short cType, int param, int flags);
    virtual ~BaseConsumable();

    ConsumableType getConsumeType()
    {
        return cType;
    }
    int getParam()
    {
        return param;
    }

private:
    int param;
    ConsumableType cType;
};

#endif /* BASECONSUMABLE_H_ */
