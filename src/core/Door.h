/*
 * Door.h
 *
 *  Created on: 2012-12-15
 *      Author: per
 */

#ifndef DOOR_H_
#define DOOR_H_
#include "MapPoint.h"
#include "IDataStream.h"

class Map;

class Door: public MapPoint
{
public:
    Door(unsigned short x, unsigned short y, Map *map, char dir);
    void toggle();

    char faces()
    {
        return ddir;
    }
    char isClosed()
    {
        return open ? 0 : 1;
    }
    bool isOpen()
    {
        return open;
    }
    const char *getDesc();
    void getViewedBlock(IDataStream *dp);

private:
    char ddir;
    bool open;
    Map *m;
};

#endif /* DOOR_H_ */
