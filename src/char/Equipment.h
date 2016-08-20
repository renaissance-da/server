/*
 * Equipment.h
 *
 *  Created on: 2013-01-05
 *      Author: per
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_

#include "Item.h"
#include "Paths.h"

class Equipment: public Item
{
public:
    enum Slots
    {
        WEAPON = 1,
        ARMOR = 2,
        SHIELD = 3,
        HEAD = 4,
        EARRING = 5,
        NECKLACE = 6,
        L_RING = 7,
        R_RING = 8,
        L_HAND = 9,
        R_HAND = 10,
        BELT = 11,
        GREAVES = 12,
        BOOTS = 13,
        ACC = 14,
        OVERCOAT = 15,
        OVERHAT = 16,
        ACC2 = 17,
        ACC3 = 18
    };

    enum SecondaryMods
    {
        MOD_MAGIC = 1, //50mp OR 1/2 mp cost
        MOD_MIGHT = 2, //100 hp
        MOD_BLESSED = 3, //5 hit
        MOD_ABUNDANCE = 4, //2 dmg
        MOD_DEOCH = 5, //10 regen
        MOD_GRAMAIL = 6, //10 mr
        MOD_SGRIOS = 7, //20% durability
        MOD_CAIL = 8, //1 con
        MOD_GLIOCA = 9, //1 wis
        MOD_LUATHAS = 10, //1 int
        MOD_CEANNLAIDIR = 11, //1 str
        MOD_FIOSACHD = 12 //1 dex
    };
    enum PrimaryMods
    {
        GOOD = 1, FINE = 2, GRAND = 3, GREAT = 4, SHODDY = 5
    };
    enum EleMods
    {
        FIRE = 1, WATER = 2, WIND = 3, EARTH = 4
    };

    Equipment(unsigned int id, unsigned short qty, unsigned int dur);
    ~Equipment();

    unsigned char getSlot()
    {
        return (unsigned char)baseEq->getSlot();
    }
    bool allowsGender(char gender)
    {
        return baseEq->allowsGender(gender);
    }
    bool allowsPath(Path p)
    {
        return baseEq->allowsPath(p);
    }
    Path getPath()
    {
        return baseEq->getPath();
    }
    unsigned short levelReq()
    {
        return baseEq->levelReq();
    }
    const Stats *getStats()
    {
        return stats ? stats : baseEq->getStats();
    }
    unsigned short getExtraApr()
    {
        return baseEq->getExtraApr();
    }
    unsigned short getEqpApr()
    {
        return baseEq->getEqpApr();
    }
    void decDur()
    {
        dur--;
    }
    Element getElement()
    {
        return mod ? (Element) mod : baseEq->getEle();
    }
    int getFlags()
    {
        return baseEq->getFlags();
    }
    bool isStaff();

    void randomMod();
    void setMod(int mod);
    static const char *getModName(unsigned char slot, int mod);

private:

};

#endif /* EQUIPMENT_H_ */
