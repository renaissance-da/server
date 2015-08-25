/*
 * MobInfo.h
 *
 *  Created on: May 5, 2014
 *      Author: per
 */

#ifndef MOBINFO_H_
#define MOBINFO_H_

#include <unordered_map>
#include "BaseItem.h"
#include <vector>
#include "Mob.h"
#include "element.h"

namespace mob {

class MobInfo {
public:
	struct Drop {
		BaseItem *bi;
		unsigned short rate;
		short mod;
	};

	MobInfo(int id, unsigned short level, unsigned hplo, const char *name, unsigned minDmg, unsigned maxDmg,
			unsigned exp, unsigned short apr, unsigned short mode, short eatk,
			short edef, short ac, short power, std::vector<Drop> &&dropList,
			unsigned minGold, unsigned maxGold, short mr, short dex,
			int hpmax, char size, unsigned short regen, short submode,
			std::vector<std::pair<int, short> > &&skillList);
	~MobInfo();

	static Mob *spawnById(int id, short x, short y, Map *map);

private:
	int id;
	short mode, submode;
	unsigned short level, apr, ac, power, mr, dex, regen;
	char name[21];
	unsigned minDmg, maxDmg, hplo, hpmax, exp, minGold, maxGold;
	char size;
	Element atk, def;

	std::vector<Drop> dropList;
	std::vector<std::pair<int, short> > mobSkills;

	static std::unordered_map<int, MobInfo *> infos;
};

} /* namespace mob */

#endif /* MOBINFO_H_ */
