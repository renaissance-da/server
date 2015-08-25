/*
 * Paths.h
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef PATHS_H_
#define PATHS_H_

#include <string.h>

//TODO think we only need 6 paths
//Ordered as according to country list enumerator
enum Path
{
    Peasant = 0,
    Warrior = 1,
    Rogue = 2,
    Wizard = 3,
    Priest = 4,
    Monk = 5,
    Master = 6,
    Gladiator = 7,
    Druid = 8,
    Archer = 9,
    Summoner = 10,
    Bard = 11,
    Path_Max = 12
};

struct PathInfo
{
    Path path;
    int mask;
    const char *name;
    unsigned char nameLen;
    int levelMod;
    int maxStr, maxDex, maxCon, maxInt, maxWis;
};

extern PathInfo paths[12];

#endif /* PATHS_H_ */
