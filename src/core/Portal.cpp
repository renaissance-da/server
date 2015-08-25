/*
 * Portal.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#include "Portal.h"
#include "Field.h"
#include <assert.h>

Portal::Portal(unsigned short x, unsigned short y, unsigned short destx,
    unsigned short desty, unsigned short mDest) :
    MapPoint(x, y, mDest), xDest(destx), yDest(desty), f(0)
//mdst(mDest)
{

}

Portal::~Portal()
{

}

void Portal::makeField(int fieldId)
{
    auto fp = Field::fields.find(fieldId);
    if (fp != Field::fields.end()) {
        f = fp->second;
    }
    else {
        // Shouldn't happen
        assert(false);
    }
}
