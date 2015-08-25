/*
 * Skill.h
 *
 * Should represent a particular knowing of a skill
 *
 *  Created on: 2012-04-10
 *      Author: per
 */

#include "SkillInfo.h"
#include "defines.h"

#ifndef SKILL_H_
#define SKILL_H_

#define NUM_SKILLS 86

#include <string>
#include "IDataStream.h"
#include "Stats.h"
#include "skills.h"

class Entity;

class Skill
{
public:
    Skill(int skillId, Path p = Peasant, char level = 0, unsigned int uses = 0);
    virtual ~Skill();

    std::string getName()
    {
        return base->name;
    }
    unsigned short getLevel()
    {
        return level;
    }
    unsigned short getMaxlevel()
    {
        return maxlevel;
    }
    int getId()
    {
        return id;
    }

    void getSkillInfo(IDataStream *dp);
    SkillInfo *getBase()
    {
        return base;
    }
    bool onCd();
    void decCd();
    virtual void used(bool imp = false, bool forceMin = true);
    bool isAssail();
    char getAnim();
    char getSound()
    {
        return base->sound;
    }
    int getUseCount()
    {
        return uses / 100;
    }
    bool canLevel()
    {
        return base->rate >= 0 && 100 * base->rate <= uses && level < maxlevel;
    }
    void levelUp();
    bool canLearn(const Stats *st, short level);
    TargetType getTargetType()
    {
        return base->tgt;
    }
    int getFlags()
    {
        return base->flags;
    }
    int getElemMod()
    {
        return (base->elem);
    }
    void setCd(unsigned cd)
    {
        remCd = cd * TICKS;
    }
    unsigned getCd()
    {
        return remCd / TICKS;
    }
    virtual bool isSecret()
    {
        return false;
    }

    static Skill *getRevive()
    {
        if (rev)
            return rev;
        else
            return rev = new Skill(SK_REVIVE);
    }

private:
    static Skill *rev;

    unsigned short level, maxlevel;
    int id;
    unsigned int remCd;
    unsigned int uses;

    SkillInfo *base;

};

#endif /* SKILL_H_ */
