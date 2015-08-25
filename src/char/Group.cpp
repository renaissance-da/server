/*
 * Group.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: per
 */

#include "Group.h"
#include "DataService.h"

std::atomic_uint Group::groupIds(0);

Group::Group()
{
    groupId = groupIds++;
}

Group::~Group()
{
    //Clear instances? (or at least start the process)
}

/**
 * Get an instanced version of the specified map for this group.
 * \param[in] mapid The ID of the map to get an instance of.
 * \param[out] isNew When not null, isNew is set to true when
 * 			   the instance was created by this call, and false
 * 			   if it already existed.
 * \return The requested instance, or null if the mapid is invalid.
 */
Instance *Group::getInstance(unsigned short mapid, bool *isNew)
{
    Instance *r = Instance::getInstance(groupId, mapid);

    if (r) {
        if (isNew)
            *isNew = false;
    }
    else {
        Map *m = DataService::getService()->getMap(mapid);
        if (!m)
            return 0;
        r = new Instance(m, groupId);
        instances.insert(mapid);
        if (isNew)
            *isNew = true;
    }

    return r;
}
