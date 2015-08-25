/*
 * Portal.h
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef PORTAL_H_
#define PORTAL_H_

#include "MapPoint.h"
#include "Field.h"

class Portal: public MapPoint
{
public:
    Portal(unsigned short x, unsigned short y, unsigned short destx,
        unsigned short desty, unsigned short mDest);
    ~Portal();

    unsigned short destX()
    {
        return xDest;
    }
    unsigned short destY()
    {
        return yDest;
    }

    void makeField(int fieldId);
    Field *getField()
    {
        return f;
    }

private:
    unsigned short xDest, yDest; //, mdst;
    Field *f;
};

#endif /* PORTAL_H_ */
