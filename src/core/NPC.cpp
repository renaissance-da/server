/*
 * NPC.cpp
 *
 *  Created on: 2012-12-17
 *      Author: per
 */

#include "NPC.h"
#include <string.h>
#include "npc_bindings.h"

NPC::NPC(unsigned short x, unsigned short y, Map *map, std::string name,
    unsigned short apr, unsigned short dir) :
    Entity(x, y, map, name), apr(apr)
{
    setDir(dir);
}

NPC::~NPC()
{
}

void NPC::getViewedBlock(IDataStream *dp)
{
    //For Riona seems to be 0003 0003 0000174b 4204 (6 zeros) 2 5 R i o n a
    dp->appendShort(getX());
    dp->appendShort(getY());
    dp->appendInt(getOid());
    dp->appendShort(apr);
    dp->appendInt(0);
    dp->appendByte(getDir());
    dp->appendByte(0);
    dp->appendByte(2);
    dp->appendString(name.length(), name.c_str());
}

void NPC::talked(Entity *who, std::string text, unsigned char channel)
{
    if (who->getType() == Entity::E_CHARACTER)
        talkResponse((Character *) who, this, text);
}

/**
 * \brief Attempts to give an item to an NPC
 *
 * Attempts to give an item to an NPC. The NPC may or may not take the item.
 * \param[in] Who The session of the character giving the item.
 * \param[in] itm The item being given to the NPC
 * \return True if the NPC accepted the item. If the item was accepted,
 * the pointer to itm should be considered invalid.
 */
bool NPC::giveItem(CharacterSession *who, Item *itm)
{
    //TODO implement
    return false;
}
