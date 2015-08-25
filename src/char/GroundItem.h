/*
 * GroundItem.h
 *
 *  Created on: 2013-01-05
 *      Author: per
 */

#ifndef GROUNDITEM_H_
#define GROUNDITEM_H_

#include "Entity.h"
#include "IDataStream.h"
#include "Item.h"

#include <vector>

class GroundItem: public Viewable
{
public:
    GroundItem(Item *item, unsigned short map, unsigned short x,
        unsigned short y, unsigned int protectionTime = 0,
        std::vector<unsigned int> *lootList = 0);
    GroundItem(unsigned int goldAmt, unsigned short map, unsigned short x,
        unsigned short y, unsigned int protectionTime = 0,
        std::vector<unsigned int> *lootList = 0);
    virtual ~GroundItem();

    void getViewedBlock(IDataStream *dp);
    std::string getName()
    {
        return isGold() ? "Gold" : item->getName();
    }
    Item *getItem()
    {
        return item;
    }
    bool isGold()
    {
        return amt;
    }
    unsigned int getGoldAmt()
    {
        return amt;
    }
    bool canLoot(unsigned int id);

private:
    GroundItem(unsigned short map, unsigned short x, unsigned short y,
        unsigned int protectionTime, std::vector<unsigned int> *lootList);

    Item *item;
    unsigned int amt, protectionTime;

    unsigned int *lootList;
    unsigned int nLooters;
};

#endif /* GROUNDITEM_H_ */
