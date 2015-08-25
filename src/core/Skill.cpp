/*
 * Skill.cpp
 *
 *  Created on: 2012-04-10
 *      Author: per
 */

#include "Skill.h"
#include <stdio.h>
#include "defines.h"
#include "element.h"
#include "random.h"
#include "Entity.h"
#include <assert.h>
#include "config.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using std::string;

Skill *Skill::rev = 0;

Skill::Skill(int skillId, Path p, char level, unsigned int uses) :
    level(level), id(skillId), remCd(0), uses(uses)
{
    base = SkillInfo::getById(skillId);
    assert(base);
    maxlevel = base->getMaxLevel(p);
}

Skill::~Skill()
{
}

void Skill::getSkillInfo(IDataStream *dp)
{
    dp->appendByte(base->icon);
    if (maxlevel) {
        char buffer[128];
        unsigned char l = snprintf(buffer, 128, "%s (Lev:%hu/%hu)", base->name,
            level, maxlevel);
        dp->appendString(l, buffer);
    }
    else
        dp->appendString(base->nameLen, base->name);

}

bool Skill::onCd()
{
    return remCd != 0;
}

void Skill::decCd()
{
    if (remCd > 0) {
        remCd--;
    }
}

char Skill::getAnim()
{
    return base->anim;
}

void Skill::used(bool imp, bool forceMin)
{
    remCd = (!base->cd && forceMin ? 1 : base->cd) * TICKS;
    if (imp)
        uses += config::skillMod;
}

bool Skill::canLearn(const Stats *st, short level)
{
    return base->str <= st->getStr() && base->dex <= st->getDex()
        && base->con <= st->getCon() && base->int_ <= st->getInt()
        && base->wis <= st->getWis() && base->level <= level;
}

bool Skill::isAssail()
{
    return (id == SK_ASSAIL || id == SK_DOUBLE_PUNCH || id == SK_ASSAULT
        || id == SK_CLOBBER || id == SK_WALLOP || id == SK_MOB_ASSAIL
        || id == SK_LONG_STRIKE || id == SK_THRASH || id == SK_TRIPLE_KICK
        || id == SK_MIDNIGHT_SLASH);
}

void Skill::levelUp()
{
    if (level < maxlevel) {
        level++;
    }
    uses = 0;
}
