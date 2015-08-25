/*
 * Consumable.h
 *
 *  Created on: 2013-01-07
 *      Author: per
 */

#ifndef CONSUMABLE_H_
#define CONSUMABLE_H_

#include "Item.h"
#include "BaseConsumable.h"
#include "Skill.h"
#include "Combat.h"

class Consumable: public Item
{
public:

    enum Scrolls
    {
        MILETH = 0
    };

    Consumable(unsigned int id, unsigned short qty, unsigned int dur);
    virtual ~Consumable();

    BaseConsumable::ConsumableType getType()
    {
        return baseCon->getConsumeType();
    }
    int getParam()
    {
        return baseCon->getParam();
    }

    template<typename E>
    bool use(E *e);
};

template<typename E>
bool Consumable::use(E *e)
{
    int param = baseCon->getParam();

    switch (getType()) {
    case BaseConsumable::FOOD:
        e->heal(param, false);
        break;
    case BaseConsumable::HP_POTION:
        if (param < 0 && (unsigned) (-param) >= e->getHp())
            e->damage(0, e->getHp() - 1);
        else
            e->heal(param, false);
        break;
    case BaseConsumable::REVIVE:
        return Combat::useSkill(e, Skill::getRevive());
    case BaseConsumable::MP_POTION:
        e->addMp(param);
        break;
    case BaseConsumable::SCROLL:
        if (e->getMap()->noScrolls())
            return false;

        switch (param) {
        case MILETH:
            return DataService::getService()->tryChangeMap(e, 3006, 9, 7, 4);
        default:
            break;
        }
        break;
    case BaseConsumable::CURES:
        return e->removeEffect((StatusEffect::Kind) param);
    case BaseConsumable::BUFFS:
        return e->addStatusEffect(param);
    }

    return true;
}

#endif /* CONSUMABLE_H_ */
