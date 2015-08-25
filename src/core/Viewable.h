/*
 * Viewable.h
 *
 *  Created on: Feb 1, 2014
 *      Author: per
 */

#ifndef VIEWABLE_H_
#define VIEWABLE_H_

#include "MapPoint.h"
#include "IDataStream.h"
#include <atomic>

class Viewable: public MapPoint
{
public:
    Viewable(int x, int y, unsigned short m);
    virtual ~Viewable();

    virtual void getViewedBlock(IDataStream *d) = 0;
    virtual bool extendedViewType();

    unsigned int getOid()
    {
        return oid;
    }
    virtual short getHideLevel();

protected:
    //TODO for now the max is 10M (a lot!) but there should be no max
    static const unsigned int maxOid;

private:
    unsigned int oid;
    static std::atomic_uint oid_alloc;
};

#endif /* VIEWABLE_H_ */
