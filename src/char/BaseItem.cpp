/*
 * BaseItem.cpp
 *
 *  Created on: 2013-01-03
 *      Author: per
 */

#include "BaseItem.h"
#include <string.h>
#include <algorithm>
#include "defines.h"
#include <assert.h>

std::map<unsigned int, BaseItem *> BaseItem::items;

BaseItem::BaseItem(const char *name, unsigned short groundApr,
    unsigned short wgt, unsigned int val, unsigned short maxStack,
    unsigned int id, bool mod, ItemType type, unsigned int flags) :
    type(type), groundApr(groundApr | 0x8000), weight(wgt), maxStack(maxStack), id(
        id), val(val), base_flags(flags), canmod(mod)
{
    strcpy(this->name, name);
    nameLen = strlen(this->name);

    assert(!items.count(id));
    items[id] = this;

    maxDur = 0;
}

BaseItem::~BaseItem()
{
    items.erase(id);
}

void BaseItem::clear()
{
    using std::pair;
    std::for_each(items.begin(), items.end(),
        [&](pair<unsigned int, BaseItem *> bi) {delete bi.second;});
    items.clear();
}

BaseItem *BaseItem::getById(unsigned int id)
{
    auto it = items.find(id);
    if (it != items.end())
        return it->second;
    else
        return 0;
}
