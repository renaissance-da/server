/*
 * BaseEffect.cpp
 *
 *  Created on: 2013-05-09
 *      Author: per
 */

#include "BaseEffect.h"
#include <string.h>
#include "defines.h"
#include <assert.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

std::map<int, BaseEffect *> BaseEffect::effects;

BaseEffect::BaseEffect(int id, int kind, int duration, unsigned short icon,
    const char *received, const char *ended, int param1, int param2) :
    id(id), kind(kind), duration(duration), param1(param1), param2(param2), icon(
        icon)
{
    strcpy(this->received, received);
    strcpy(this->ended, ended);

    assert(!effects.count(id));
    effects[id] = this;

    if (kind == 1) {
        snprintf(conflict, 128, "Another curse afflicts thee(%s).", received);
    }
    else {
        strcpy(conflict, "You already cast that spell.");
    }
}

BaseEffect::~BaseEffect()
{

}

BaseEffect *BaseEffect::getById(int id)
{
    auto it = effects.find(id);
    if (it == effects.end())
        return 0;
    return it->second;
}

