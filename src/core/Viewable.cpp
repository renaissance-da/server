/*
 * Viewable.cpp
 *
 *  Created on: Feb 1, 2014
 *      Author: per
 */

#include <Viewable.h>
#include <assert.h>

std::atomic_uint Viewable::oid_alloc(0);
const unsigned int Viewable::maxOid = 10000000;

Viewable::Viewable(int x, int y, unsigned short m) :
    MapPoint(x, y, m), oid(++oid_alloc)
{
    assert(oid < maxOid);
}

Viewable::~Viewable()
{
    //TODO free oid
}

short Viewable::getHideLevel()
{
    return 0;
}

bool Viewable::extendedViewType()
{
    return false;
}
