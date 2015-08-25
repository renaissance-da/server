/*
 * NPC.h
 *
 *  Created on: 2012-12-17
 *      Author: per
 */

#ifndef NPC_H_
#define NPC_H_

#include "Entity.h"
#include "IDataStream.h"
#include "Character.h"

class NPC: public Entity
{
public:
    NPC(unsigned short x, unsigned short y, Map *map, std::string name,
        unsigned short apr, unsigned short dir);
    virtual ~NPC();

    virtual EntityType getType()
    {
        return Entity::E_NPC;
    }
    void getViewedBlock(IDataStream *dp);
    short getApr()
    {
        return apr;
    }
    void talked(Entity *who, std::string text, unsigned char channel = 0)
        override;bool giveItem(CharacterSession *who, Item *itm);

private:
    unsigned short apr;

};

#endif /* NPC_H_ */
