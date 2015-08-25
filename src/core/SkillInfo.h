/*
 * SkillInfo.h
 *
 * Represents generic information about a skill
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef SKILLINFO_H_
#define SKILLINFO_H_

#include "Paths.h"
#include "skills.h"
#include <map>
#include <vector>
#include "BaseEffect.h"

class SkillInfo
{
public:

    struct Req
    {
        int id;
        short lev;
    };

    unsigned int getMaxLevel(Path p);

    static SkillInfo *getById(unsigned int id);
    static void addSkill(SkillInfo *si, unsigned int sid);
    static void clear();

    char maxLevel[6];
    unsigned int cd;
    unsigned char sound;
    unsigned char anim;
    short animLen;
    unsigned char icon;
    unsigned short effect, effectSelf;
    int rate;
    unsigned int flags;
    TargetType tgt;

    short level, str, dex, con, int_, wis, item1Qty, item2Qty, item3Qty, path,
        item1mod, item2mod, item3mod;
    int gold, item1, item2, item3, mp;
    short elem;
    BaseEffect *buff;

    char name[128];
    char nameLen;

    std::vector<Req> reqs;

private:
    static std::map<unsigned int, SkillInfo *> allSkills;

};

#endif /* SKILLINFO_H_ */
