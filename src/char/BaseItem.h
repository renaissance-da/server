/*
 * BaseItem.h
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#ifndef BASEITEM_H_
#define BASEITEM_H_

#include <map>

class BaseItem
{
public:

    enum ItemType
    {
        PLAIN, EQUIP, CONSUME
    };

    BaseItem(const char *name, unsigned short groundApr, unsigned short wgt,
        unsigned int val, unsigned short maxStack, unsigned int id, bool mod,
        ItemType type, unsigned int flags);
    virtual ~BaseItem();

    const char *getName()
    {
        return name;
    }
    unsigned short getNameLen()
    {
        return nameLen;
    }
    unsigned short getWeight()
    {
        return weight;
    }
    unsigned short getMaxStack()
    {
        return maxStack;
    }
    unsigned short getApr()
    {
        return groundApr;
    }
    ItemType getType()
    {
        return type;
    }
    unsigned int getMaxDur()
    {
        return maxDur;
    }
    unsigned int getId()
    {
        return id;
    }
    unsigned int getVal()
    {
        return val;
    }
    unsigned int getBaseFlags()
    {
        return base_flags;
    } // 1 = soulbound, 2 = perish
    bool modify()
    {
        return canmod;
    }

    static BaseItem *getById(unsigned int id);
    static void clear();

protected:
    ItemType type;
    int maxDur;

private:
    static std::map<unsigned int, BaseItem *> items;

    char name[128];
    unsigned short nameLen;
    unsigned short groundApr;
    unsigned short weight;
    unsigned char maxStack;
    unsigned int id;
    unsigned int val;
    unsigned int base_flags;
    bool canmod;
};

#endif /* BASEITEM_H_ */
