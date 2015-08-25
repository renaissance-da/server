/*
 * SkillInfo.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#include "SkillInfo.h"
#include <algorithm>

std::map<unsigned int, SkillInfo *> SkillInfo::allSkills;

void SkillInfo::clear()
{
    using std::pair;
    std::for_each(allSkills.begin(), allSkills.end(),
        [&](pair<unsigned int, SkillInfo *> sk) {delete sk.second;});
    allSkills.clear();
}

unsigned int SkillInfo::getMaxLevel(Path p)
{
    switch (p) {
    case Peasant:
        return 0;
    case Warrior:
    case Monk:
    case Rogue:
    case Wizard:
    case Priest:
        return maxLevel[p - 1];
    default:
        return maxLevel[5];
    }
}

void SkillInfo::addSkill(SkillInfo *si, unsigned int sid)
{
    allSkills[sid] = si;
}

SkillInfo *SkillInfo::getById(unsigned int id)
{
    std::map<unsigned int, SkillInfo *>::iterator it = allSkills.find(id);
    if (it != allSkills.end()) {
        return it->second;
    }
    return 0;
}

