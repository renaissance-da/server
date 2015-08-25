/*
 * MapPoint.cpp
 *
 *  Created on: 2012-12-23
 *      Author: per
 */

#include "MapPoint.h"

MapPoint::MapPoint(unsigned short x, unsigned short y, unsigned short map) :
    x(x), y(y), map(map)
{

}

MapPoint::~MapPoint()
{
    // TODO Auto-generated destructor stub
}

unsigned int MapPoint::dist(const MapPoint *other)
{
    if (map == other->map)
        return MH_DIST(x, other->x, y, other->y);

    else
        return 0x7fffffff;
}
