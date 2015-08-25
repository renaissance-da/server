/*
 * BaseEquipment.h
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#ifndef BASEEQUIPMENT_H_
#define BASEEQUIPMENT_H_

#include "BaseItem.h"
#include "Stats.h"
#include "element.h"
#include "Paths.h"

class BaseEquipment: public BaseItem
{
public:
public:
    BaseEquipment(const char *name, unsigned short groundApr,
        unsigned short wgt, unsigned int value, unsigned int id,
        unsigned short slot, unsigned char gender, unsigned short level,
        unsigned short eqpApr, unsigned short extraApr, int dur, short path,
        unsigned int wMin, unsigned int wMax, short ac, int hp, int mp,
        short hit, short dmg, short mr, short str, short con, short dex,
        short int_, short wis, short ele, short regen, int flags, bool modify,
        int lMin, int lMax, unsigned int bflags);
    virtual ~BaseEquipment();

    const Stats *getStats()
    {
        return &stats;
    }
    bool allowsGender(char gender);
    unsigned short getSlot()
    {
        return slot;
    }
    unsigned short levelReq()
    {
        return level;
    }
    unsigned short getEqpApr()
    {
        return eqpApr;
    }
    unsigned short getExtraApr()
    {
        return extraApr;
    }
    Element getEle()
    {
        return ele;
    }
    bool allowsPath(Path p);
    bool canModify()
    {
        return canMod;
    }
    int getFlags()
    {
        return flags;
    } //1=monk 2=surigam 4=two handed
    Path getPath()
    {
        return path;
    }

private:
    Stats stats; //applied when equipped/unequipped
    unsigned short level, slot;
    char gender;
    unsigned short eqpApr, extraApr;
    Path path;
    Element ele;
    int flags;
    bool canMod;
};

#endif /* BASEEQUIPMENT_H_ */
