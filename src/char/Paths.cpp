/*
 * Paths.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#include "Paths.h"


PathInfo paths[12] = {
    { Peasant, 0x000, "Peasant", (unsigned char)strlen("Peasant"), 600, 100, 100, 100, 100, 100 },
    { Warrior, 0x001, "Warrior", (unsigned char)strlen("Warrior"), 630, 215, 150, 180, 100, 100 },
    { Rogue, 0x002, "Rogue", (unsigned char)strlen("Rogue"), 570, 180, 215, 150, 100, 100 },
    { Wizard, 0x004, "Wizard", (unsigned char)strlen("Wizard"), 618, 100, 100, 150, 215, 180 },
    { Priest, 0x008, "Priest", (unsigned char)strlen("Priest"), 600, 100, 100, 150, 180, 215 },
    { Monk, 0x010, "Monk", (unsigned char)strlen("Monk"), 588, 180, 100, 215, 150, 100 },
    { Master, 0x020, "Master", (unsigned char)strlen("Master"), 0 },
    { Gladiator, 0x040, "Gladiator", (unsigned char)strlen("Gladiator"), 963 },
    { Druid, 0x080, "Druid", (unsigned char)strlen("Druid"), 918 },
    { Archer, 0x100, "Archer", (unsigned char)strlen("Archer"), 954 },
    { Summoner, 0x200, "Summoner", (unsigned char)strlen("Summoner"), 900 },
    { Bard, 0x400, "Bard", (unsigned char)strlen("Bard"), 927 }
};
