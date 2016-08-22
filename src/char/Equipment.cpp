/*
 * Equipment.cpp
 *
 *  Created on: 2013-01-05
 *      Author: per
 */

#include "Equipment.h"
#include "random_engines.h"
#include <assert.h>

Equipment::Equipment(unsigned int id, unsigned short qty, unsigned int dur) :
    Item(id, qty, dur)
{

}

Equipment::~Equipment()
{

}

void Equipment::randomMod()
{
    std::uniform_real_distribution<double> st_uniform_dist;
    std::uniform_int_distribution<int> elem_dist(1, 4);
    std::uniform_int_distribution<int> secondary_dist(1, 12);
    
    if (mod) {
        delete stats;
        mod = 0;
        stats = 0;
        weight = baseEq->getWeight();
        maxDur = baseEq->getMaxDur();
    }
    int rmod;
    if (getSlot() == WEAPON || getSlot() == ARMOR) {
        //Primary mods favour the lower mods
        float r = st_uniform_dist(generator());
        if (r < .5) {
            rmod = 1;
        }
        else if (r < .75) {
            rmod = 2;
        }
        else if (r < .915) {
            rmod = 3;
        }
        else {
            rmod = 4;
        }
        setMod(rmod);
    }
    //ele mods
    else if (getSlot() == BELT || getSlot() == NECKLACE) {
        setMod(elem_dist(generator()));
    }
    //Secondary mods
    else {
        setMod(secondary_dist(generator()));
    }
}

const char *Equipment::getModName(unsigned char slot, int mod)
{
    if (slot == WEAPON || slot == ARMOR) {
        switch (mod) {
        case GOOD:
            return "Good";
        case FINE:
            return "Fine";
        case GRAND:
            return "Grand";
        case GREAT:
            return "Great";
        case SHODDY:
            return "Shoddy";
        }
    }
    else if (slot == BELT || slot == NECKLACE) {
        switch (mod) {
        case FIRE:
            return "Fire";
        case WATER:
            return "Water";
        case EARTH:
            return "Earth";
        case WIND:
            return "Wind";
        }
    }
    else {
        switch (mod) {
        case MOD_MAGIC:
            return "Magic";
        case MOD_MIGHT:
            return "Might";
        case MOD_BLESSED:
            return "Blessed";
        case MOD_ABUNDANCE:
            return "Abundance";
        case MOD_DEOCH:
            return "Deoch";
        case MOD_GRAMAIL:
            return "Gramail";
        case MOD_SGRIOS:
            return "Sgrios";
        case MOD_CAIL:
            return "Cail";
        case MOD_GLIOCA:
            return "Glioca";
        case MOD_LUATHAS:
            return "Luathas";
        case MOD_CEANNLAIDIR:
            return "Ceannlaidir";
        case MOD_FIOSACHD:
            return "Fiosachd";
        default:
            assert(!"reachable");
            break;
        }
    }
    return "";
}

void Equipment::setMod(int mod)
{
    if (this->mod) {
        delete stats;
        weight = baseEq->getWeight();
        maxDur = baseEq->getMaxDur();
    }
    this->mod = mod;
    const Stats *base = baseEq->getStats();
    Stats *st = new Stats;
    *st = *base;
    if (getSlot() == WEAPON || getSlot() == ARMOR) {
        if (getSlot() == WEAPON) {
            st->setMaxAtk(st->getAtkMax() + 2 * mod);
            st->setMinAtk(st->getAtkMin() + 2 * mod);
        }
        else {
            if (mod < SHODDY)
                st->setAc(st->getAc() - 2 * mod);
            else
                st->setAc(st->getAc() + 4);
        }
        if (mod < SHODDY)
            weight = (weight * (5 - mod) + 4) / 5;
        switch (mod) {
        case GOOD:
            name = std::string("Good ") + baseEq->getName();
            break;
        case FINE:
            name = std::string("Fine ") + baseEq->getName();
            break;
        case GRAND:
            name = std::string("Grand ") + baseEq->getName();
            break;
        case GREAT:
            name = std::string("Great ") + baseEq->getName();
            break;
        case SHODDY:
            name = std::string("Shoddy ") + baseEq->getName();
            break;
        }
    }
    else if (getSlot() == BELT || getSlot() == NECKLACE) {
        switch (mod) {
        case FIRE:
            name = std::string("Fire ") + baseEq->getName();
            break;
        case WATER:
            name = std::string("Water ") + baseEq->getName();
            break;
        case EARTH:
            name = std::string("Earth ") + baseEq->getName();
            break;
        case WIND:
            name = std::string("Wind ") + baseEq->getName();
            break;
        }
    }
    else {
        switch (mod) {
        case MOD_MAGIC:
            name = std::string("Magic ") + baseEq->getName();
            st->magicMod();
            break;
        case MOD_MIGHT:
            name = std::string("Might ") + baseEq->getName();
            st->incHp(100);
            break;
        case MOD_BLESSED:
            name = std::string("Blessed ") + baseEq->getName();
            st->incHit(5);
            break;
        case MOD_ABUNDANCE:
            name = std::string("Abundance ") + baseEq->getName();
            st->incDmg(2);
            break;
        case MOD_DEOCH:
            name = std::string("Deoch ") + baseEq->getName();
            st->incRegen(10);
            break;
        case MOD_GRAMAIL:
            name = std::string("Gramail ") + baseEq->getName();
            st->incMr(1);
            break;
        case MOD_SGRIOS:
            name = std::string("Sgrios ") + baseEq->getName();
            maxDur = (maxDur * 6) / 5;
            dur = maxDur;
            break;
        case MOD_CAIL:
            name = std::string("Cail ") + baseEq->getName();
            st->incCon(1);
            break;
        case MOD_GLIOCA:
            name = std::string("Glioca ") + baseEq->getName();
            st->incWis(1);
            break;
        case MOD_LUATHAS:
            name = std::string("Luathas ") + baseEq->getName();
            st->incInt(1);
            break;
        case MOD_CEANNLAIDIR:
            name = std::string("Ceannlaidir ") + baseEq->getName();
            st->incStr(1);
            break;
        case MOD_FIOSACHD:
            name = std::string("Fiosachd ") + baseEq->getName();
            st->incDex(1);
            break;
        default:
            assert(!"reachable");
            break;
        }
    }
    stats = st;
}

bool Equipment::isStaff()
{
    return false;
}
