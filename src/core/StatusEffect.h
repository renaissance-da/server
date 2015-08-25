/*
 * StatusEffect.h
 *
 *  Created on: 2013-03-09
 *      Author: per
 */

#ifndef STATUSEFFECT_H_
#define STATUSEFFECT_H_

#include "Skill.h"
#include "Stats.h"
#include "BaseEffect.h"
#include "element.h"

class Entity;

class StatusEffect
{
public:
    enum Kind
    {
        ARMOR = 0, //adds param1 to ac (param is usually negative)
        CURSE = 1, //adds param1 to ac (param is usually positive), and adds param2 to mr(u. negative)
        POISON = 2, //deals param1 dps (if param1<0, |param1|% total hp dps), animates as param2
        BLIND = 3, //as the name suggests
        SLEEP = 4, //as the name suggests. param1 is used as the animation
        STUN = 5, //prevents movement. param1 is used as the animation
        FREEZE = 6, //prevents action. param1 is used as the animation
        REDUCTION = 7, //reduces incoming damage by param1%
        SLOW = 8, //reduces the action speed of mobs by param1%
        ELEMENT = 9, //changes the element of mobs to param1
        INVULNERABILITY = 10, //ignores all damage
        STRENGTH = 11, //Increases strength by param1
        REGEN = 12, //Increases regen by param1
        DODGE = 13, //Increases dodge rate by param1
        MAGIC_RESIST = 14, //Increases mr by param1*10%
        SILENCE = 15, //Prevents activation of secrets. param1 is an animation
        DOOM = 16, //Prevents action, character dies on expiration. param1 is an animation
        SLAN = 17, //Strengthens unarmed assails TODO check if slan does this
        FAS = 18, //Increases elemental level to param1
        SIGHT = 19, //Allows viewing of creatures with concealment param1 or less
        CONCEAL = 20, //Prevents view by creatures with sight less than param1
        HIT = 21, //param1 is bonus hit
        DMG = 22, //param1 is bonus dmg
        ATTACK_REFLECT = 23, //reflect chance is param1, reflects any ability which can be deflected
        MAGIC_REFLECT = 24, //reflect chance is param1, reflects any single target spell
        SE_KINDS = 25
    };

    //StatusEffect(int skid, int dur);
    StatusEffect(int buffid);
    StatusEffect(int buffid, int dur);
    StatusEffect(BaseEffect *base);
    StatusEffect(BaseEffect *base, int dur);
    ~StatusEffect();

    float modDmg();
    void getChange(Stats &s);
    unsigned char getDur();
    int getId() const
    {
        return base->id;
    }
    int getTickDuration() const
    {
        return duration;
    }
    short getIcon()
    {
        return base->icon;
    }
    int getVal()
    {
        return base->param1;
    }
    const char *getReceivedMsg()
    {
        return base->received;
    }
    const char *getEndedMsg()
    {
        return base->ended;
    }
    const char *getConflictMsg()
    {
        return base->conflict;
    }

    int tick(Entity *c);

private:
    BaseEffect *base;
    int duration;
};

#endif /* STATUSEFFECT_H_ */
