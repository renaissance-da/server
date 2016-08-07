/*
 * Group.h
 *
 *  Created on: Sep 14, 2014
 *      Author: per
 */

#ifndef GROUP_H_
#define GROUP_H_

#include <list>
#include "Instance.h"
#include <atomic>
#include <set>
#include <mutex>

class Character;

class Group: public std::list<Character *>
{
public:
    Group();
    virtual ~Group();

    Instance *getInstance(unsigned short mapid, bool *isNew);



private:
    static std::atomic_uint groupIds;
    unsigned groupId;

    std::set<unsigned short> instances;
};

#endif /* GROUP_H_ */
