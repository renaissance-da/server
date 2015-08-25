/*
 * Stats.cpp
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#include "Stats.h"

#ifdef WIN32

#else
#include <unistd.h>
#include <endian.h>
#endif

Stats::Stats() :
    maxHp(0), maxMp(0), strength(0), intelligence(0), wisdom(0), dexterity(0), constitution(
        0), hit(0), dmg(0), ac(0), mr(0), regen(0), atkMin(0), atkMax(0), atkMinL(
        0), atkMaxL(0), atkEle(None), defEle(None), dodge(0)
{
}

Stats::~Stats()
{

}

Stats Stats::operator+(Stats const &rhs)
{
    Stats result;
    result.maxHp = maxHp + rhs.maxHp;
    result.maxMp = maxMp + rhs.maxMp;
    result.strength = strength + rhs.strength;
    result.intelligence = intelligence + rhs.intelligence;
    result.wisdom = wisdom + rhs.wisdom;
    result.dexterity = dexterity + rhs.dexterity;
    result.constitution = constitution + rhs.constitution;
    result.hit = hit + rhs.hit;
    result.dmg = dmg + rhs.dmg;
    result.ac = ac + rhs.ac;
    result.mr = mr + rhs.mr;
    result.regen = regen + rhs.regen;
    result.atkMin = atkMin + rhs.atkMin;
    result.atkMax = atkMax + rhs.atkMax;
    result.dodge = dodge + rhs.dodge;
    result.atkMinL = atkMinL + rhs.atkMinL;
    result.atkMaxL = atkMaxL + rhs.atkMaxL;
    return result;
}

Stats &Stats::operator+=(Stats const &rhs)
{

    maxHp += rhs.maxHp;
    maxMp += rhs.maxMp;
    strength += rhs.strength;
    intelligence += rhs.intelligence;
    wisdom += rhs.wisdom;
    dexterity += rhs.dexterity;
    constitution += rhs.constitution;
    hit += rhs.hit;
    dmg += rhs.dmg;
    ac += rhs.ac;
    mr += rhs.mr;
    regen += rhs.regen;
    atkMin += rhs.atkMin;
    atkMax += rhs.atkMax;
    atkMinL += rhs.atkMinL;
    atkMaxL += rhs.atkMaxL;
    dodge += rhs.dodge;
    return (*this);
}

Stats operator-(Stats const &stat)
{
    Stats result;
    result.maxHp = -stat.maxHp;
    result.maxMp = -stat.maxMp;
    result.strength = -stat.strength;
    result.intelligence = -stat.intelligence;
    result.wisdom = -stat.wisdom;
    result.dexterity = -stat.dexterity;
    result.constitution = -stat.constitution;
    result.hit = -stat.hit;
    result.dmg = -stat.dmg;
    result.ac = -stat.ac;
    result.mr = -stat.mr;
    result.regen = -stat.regen;
    result.atkMin = -stat.atkMin;
    result.atkMax = -stat.atkMax;
    result.atkMinL = -stat.atkMinL;
    result.atkMaxL = -stat.atkMaxL;
    result.dodge = -stat.dodge;
    return result;
}

Stats &Stats::operator-=(Stats const &rhs)
{
    return (*this) += (-rhs);
}

Stats Stats::operator-(Stats const &rhs)
{
    return (*this) + (-rhs);
}

char Stats::getCappedAc()
{
    if (ac > 100)
        return 100;
    if (ac < -100)
        return -100;
    return ac;
}

void Stats::incHp(int amt)
{
    maxHp += amt;
}

void Stats::incMp(int amt)
{
    maxMp += amt;
}

void Stats::incAc(int amt)
{
    ac += amt;
}

float Stats::getAcMod() const
{
    //Official version, % = { (100 - ac) / 100 | ac > -95 ; 0.05 + (95 + ac) / 2000 | ac < -95
    return ac > -95 ? (100 + ac) / 100.0 : (195 + ac) / 2000.0;
}
