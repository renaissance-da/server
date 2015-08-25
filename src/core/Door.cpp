/*
 * Door.cpp
 *
 *  Created on: 2012-12-15
 *      Author: per
 */

#include "Door.h"
#include "DataService.h"
#include "defines.h"
#include "Map.h"

Door::Door(unsigned short x, unsigned short y, Map *map, char dir) :
    MapPoint(x, y, map->getId()), ddir(dir), open(false), m(map)
{
}

void Door::toggle()
{
    //Get nearby doors
    open = !open;

    switch (ddir) {
    case 1:
        //toggles doors to left and right
        for (unsigned short i = 0; i < 5; i++) {
            Door *n = m->getDoor(getX() + i - 2, getY());
            if (n) {
                n->open = open;
                m->forEachNearby(n, [&](Entity *e) {
                    e->showDoor(n);
                });
            }
        }
        break;
    case 0:
        //toggles doors above and below
        for (unsigned short i = 0; i < 5; i++) {
            Door *n = m->getDoor(getX(), getY() + i - 2);
            if (n) {
                n->open = open;
                m->forEachNearby(n, [&](Entity *e) {
                    e->showDoor(n);
                });
            }
        }
        break;
    }

    //m->unlock();
    return;
}

const char *Door::getDesc()
{
    if (open)
        return "It's open.";
    else
        return "It's closed.";
}

void Door::getViewedBlock(IDataStream *dp)
{
    dp->appendByte(getX());
    dp->appendByte(getY());
    dp->appendByte(isClosed());
    dp->appendByte(faces());
}
