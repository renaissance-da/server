/*
 * Instance.cpp
 *
 *  Created on: Sep 8, 2014
 *      Author: per
 */

#include "Instance.h"
#include <assert.h>
#include "DataService.h"

std::atomic_int Instance::nextId(20000);
std::map<std::pair<unsigned, unsigned short>, Instance *> Instance::instances;

Instance *Instance::getInstance(unsigned int groupId, unsigned short mapId)
{
    auto it = instances.find( { groupId, mapId });
    if (it != instances.end())
        return it->second;
    else
        return 0;
}

Instance::Instance(Map *base, unsigned groupId) :
    Map(base, nextId++), dispId(base->getDispId()), groupId(groupId)
{

    std::pair<unsigned, unsigned short> iid = { groupId, dispId };
    assert(!instances.count(iid));
    instances[iid] = this;

    //TODO track instances separately
    DataService::getService()->addMap(this, id);

    for (Spawner *s : spawners) {
        s->forceAll();
    }
}

Instance::~Instance()
{
    walls = 0;
    file = 0;
    instances.erase( { groupId, dispId });
}

