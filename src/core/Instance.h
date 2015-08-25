/*
 * Instance.h
 *
 *  Created on: Sep 8, 2014
 *      Author: per
 */

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include "Map.h"
#include <atomic>
#include <map>

class Instance: public Map
{
public:
    static Instance *getInstance(unsigned groupId, unsigned short mapId);
    Instance(Map *base, unsigned groupId);
    ~Instance();

    unsigned short getDispId() override
    {
        return dispId;
    }

private:

    int dispId;
    unsigned groupId;
    static std::atomic_int nextId;
    //map group id and map id to an instance
    static std::map<std::pair<unsigned, unsigned short>, Instance *> instances;
};

#endif /* INSTANCE_H_ */
