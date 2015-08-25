/*
 * GroundItem.cpp
 *
 *  Created on: 2013-01-05
 *      Author: per
 */

#include "GroundItem.h"

GroundItem::GroundItem(unsigned short map, unsigned short x, unsigned short y,
    unsigned int protectionTime, std::vector<unsigned int> *lootList) :
    Viewable(x, y, map), item(0), amt(0), protectionTime(
        protectionTime + time(NULL))
{
    if (lootList && lootList->size() > 0) {
        nLooters = lootList->size();
        this->lootList = new unsigned int[nLooters];
        for (unsigned int i = 0; i < nLooters; i++) {
            this->lootList[i] = lootList->at(i);
        }
    }
    else {
        nLooters = 0;
        this->lootList = 0;
    }
}

GroundItem::GroundItem(Item *item, unsigned short map, unsigned short x,
    unsigned short y, unsigned int protectionTime,
    std::vector<unsigned int> *lootList) :
    GroundItem(map, x, y, protectionTime, lootList)
{
    this->item = item;
}

GroundItem::GroundItem(unsigned int amtGold, unsigned short map,
    unsigned short x, unsigned short y, unsigned int protectionTime,
    std::vector<unsigned int> *lootList) :
    GroundItem(map, x, y, protectionTime, lootList)
{
    amt = amtGold;
}

GroundItem::~GroundItem()
{
    if (lootList)
        delete[] lootList;
}

void GroundItem::getViewedBlock(IDataStream *dp)
{
    dp->appendShort(getX());
    dp->appendShort(getY());
    dp->appendInt(getOid());
    if (isGold()) {
        if (amt < 10)
            dp->appendShort(0x808A);
        else if (amt < 100)
            dp->appendShort(0x8089);
        else if (amt < 1000)
            dp->appendShort(0x808D);
        else
            dp->appendShort(0x808C);
    }
    else
        dp->appendShort(item->getApr());
    dp->appendByte(0);
    dp->appendShort(0);
}

/**
 * \brief Test if something with an identifier is allowed to loot this item.
 *
 * When a GroundItem is created, it can optionally be associated with a list of
 * identifiers which can be used to control who is allowed to pick up the item.
 * This method tests if a given identifier is on the list, if applicable.
 * \param[in] id The identifier to test against this GroundItem's ID list.
 * \return True if the identifier is on this item's list, or this item has no
 * loot protection on it. False otherwise.
 */
bool GroundItem::canLoot(unsigned int id)
{
    if (lootList && protectionTime > time(NULL)) {
        for (int i = 0; i < nLooters; i++) {
            if (lootList[i] == id)
                return true;
        }
        return false;
    }

    return true;
}
