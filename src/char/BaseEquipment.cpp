/*
 * BaseEquipment.cpp
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#include "BaseEquipment.h"

BaseEquipment::BaseEquipment(const char *name, unsigned short groundApr,
    unsigned short wgt, unsigned int value, unsigned int id,
    unsigned short slot, unsigned char gender, unsigned short level,
    unsigned short eqpApr, unsigned short extraApr, int dur, short path,
    unsigned int wMin, unsigned int wMax, short ac, int hp, int mp, short hit,
    short dmg, short mr, short str, short con, short dex, short int_, short wis,
    short ele, short regen, int flags, bool modify, int lMin, int lMax,
    unsigned int bflags) :
    BaseItem(name, groundApr, wgt, value, 0, id, modify, EQUIP, bflags), level(
        level), slot(slot), gender(gender), eqpApr(eqpApr), extraApr(extraApr), path(
        (Path) path), ele((Element) ele), flags(flags), canMod(modify)
{
    maxDur = dur;

    stats.setMinAtk(wMin);
    stats.setMaxAtk(wMax);
    stats.setMinAtkL(lMin);
    stats.setMaxAtkL(lMax);
    stats.setAc(ac);
    stats.setMaxHp(hp);
    stats.setMaxMp(mp);
    stats.setHit(hit);
    stats.setDmg(dmg);
    stats.setMr(mr);
    stats.setStr(str);
    stats.setCon(con);
    stats.setDex(dex);
    stats.setInt(int_);
    stats.setWis(wis);
    stats.setRegen(regen);
}

BaseEquipment::~BaseEquipment()
{
}

bool BaseEquipment::allowsGender(char gender)
{
    return (this->gender == 0 || this->gender == gender);
}

bool BaseEquipment::allowsPath(Path p)
{
    switch (path) {
    case Peasant:
        return true;
    case Warrior:
    case Monk:
    case Rogue:
    case Priest:
    case Wizard:
        return p == path;
    default:
        return false; //TODO implement
    }
}
