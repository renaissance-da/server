/*
 * StatusEffect.cpp
 *
 *  Created on: 2013-03-09
 *      Author: per
 */

#include "StatusEffect.h"
#include "skills.h"
#include "Entity.h"
#include "defines.h"
#include <assert.h>

StatusEffect::StatusEffect(BaseEffect *base) :
    base(base)
{
    assert(base);
    duration = TICKS * base->duration;
}

StatusEffect::StatusEffect(BaseEffect *base, int dur) :
    base(base), duration(dur)
{
    assert(base);
}

StatusEffect::StatusEffect(int buffid)
{
    base = BaseEffect::getById(buffid);
    assert(base);
    duration = TICKS * base->duration;
}

StatusEffect::StatusEffect(int buffid, int dur)
{
    base = BaseEffect::getById(buffid);
    assert(base);
    duration = dur;
}

StatusEffect::~StatusEffect()
{
}

void StatusEffect::getChange(Stats &s)
{
    switch (base->kind) {
    case ARMOR:
        s.setAc(base->param1);
        break;
    case CURSE:
        s.setAc(base->param1);
        s.setMr(base->param2);
        break;
    case STRENGTH:
        s.setStr(base->param1);
        break;
    case REGEN:
        s.setRegen(base->param1);
        break;
    case DODGE:
        s.setDodge(base->param1);
        break;
    case MAGIC_RESIST:
        s.setMr(base->param1);
        break;
    case HIT:
        s.setHit(base->param1);
        break;
    case DMG:
        s.setDmg(base->param1);
        break;
    default:
        //TODO add sight levels and elemental info to status
        break;
    }
}

int StatusEffect::tick(Entity *c)
{
    //process dots
    if (--duration % TICKS == 0) {
        if (base->kind == POISON) {
            int dmg = base->param1;
            if (dmg < 0)
                dmg = (-c->getMaxHp() * dmg) / 100;
            if ((unsigned) dmg < c->getHp())
                c->damage(0, dmg);
        }

        switch (base->kind) {
        case POISON:
            if (duration > 0)
                c->playEffect(base->param2, 100);
            break;
        case SLEEP:
        case STUN:
        case FREEZE:
        case SILENCE:
        case DOOM:
            if (duration > 0)
                c->playEffect(base->param1, 100);
            break;
        default:
            break;
        }
    }

    return duration;
}

unsigned char StatusEffect::getDur()
{
    unsigned char dur = (duration / TICKS + 19) / 20;
    if (dur > 6)
        return 6;
    return dur;
}

float StatusEffect::modDmg()
{
    if (base->kind == REDUCTION)
        return (100 - base->param1) / 100.0;
    else if (base->kind == INVULNERABILITY)
        return 0.0;
    else
        return 1.0;
}
