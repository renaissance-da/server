/*
 * MobInfo.cpp
 *
 *  Created on: May 5, 2014
 *      Author: per
 */

#include <MobInfo.h>
#include <assert.h>
#include "Map.h"
#include "Equipment.h"
#include "random.h"
#include "DataService.h"

namespace mob {

std::unordered_map<int, MobInfo *> MobInfo::infos;

MobInfo::MobInfo(int id, unsigned short level, unsigned hplo, const char *name, unsigned minDmg, unsigned maxDmg,
		unsigned exp, unsigned short apr, unsigned short mode, short eatk,
		short edef, short ac, short power, std::vector<Drop> &&dropList,
		unsigned minGold, unsigned maxGold, short mr, short dex,
		int hpmax, char size, unsigned short regen, short submode,
		std::vector<std::pair<int, short> > &&skillList):
				id(id),
				mode(mode),
				submode(submode),
				level(level),
				apr(apr),
				ac(ac),
				power(power),
				mr(mr),
				dex(dex),
				regen(regen),
				minDmg(minDmg),
				maxDmg(maxDmg),
				hplo(hplo),
				hpmax(hpmax),
				exp(exp),
				minGold(minGold),
				maxGold(maxGold),
				size(size),
				dropList(std::move(dropList)),
				mobSkills(std::move(skillList))
{
	strcpy(this->name, name);

	if (eatk < 0 || eatk >= N_ELEMENTS)
		atk = N_ELEMENTS;
	else
		atk = (Element)eatk;
	if (edef < 0 || edef >= N_ELEMENTS)
		def = N_ELEMENTS;
	else
		def = (Element)edef;

	assert(!infos.count(id));
	infos[id] = this;
}

MobInfo::~MobInfo() {
	infos.erase(id);
}

/**
 * \brief Spawn a mob with the given ID
 *
 * Spawns a mob using the MobInfo identified by the given ID. The mob is
 * inserted into the given map. If either x or y are negative, the mob
 * is inserted randomly.
 * \param[in] id The unique identifier for the kind of mob to be spawned.
 * \param[in] x The x coordinate of the mob.
 * \param[in] y The y coordinate of the mob.
 * \param[in] map The map to spawn this mob on.
 * \returns A mob, initialized according to the ID given, with its map set to
 * the map given. If the mob ID or map ID are invalid, returns 0.
 */
Mob *MobInfo::spawnById(int id, short x, short y, Map *map)
{

	auto it = infos.find(id);
	if (it == infos.end())
		return 0;

	MobInfo &mi = *(it->second);

	Element a, d;
	a = mi.atk == N_ELEMENTS ? (Element)(random()%4 + 1) : mi.atk;
	d = mi.def == N_ELEMENTS ? (Element)(random()%4 + 1) : mi.def;
	Mob *mob = new Mob(x, y, map, mi.apr, mi.id, mi.hplo + drandom()*(mi.hpmax-mi.hplo),
				mi.minDmg, mi.maxDmg, mi.exp, mi.name, mi.mode, mi.level, a, d, mi.ac,
				mi.power, mi.mr, mi.dex, mi.size == 's', mi.regen, mi.submode);
	for (Drop &drop : mi.dropList) {
		if (random() % 1000 < drop.rate) {
			Item *itm = new Item(drop.bi);
			if (drop.mod < 0)
				((Equipment *)itm)->randomMod();
			else if (drop.mod > 0)
				((Equipment *)itm)->setMod(drop.mod);
			mob->giveItem(itm);
		}
	}
	mob->giveGold(mi.minGold + drandom() * (mi.maxGold - mi.minGold));
	mob->setSkills(mi.mobSkills);

	if (x < 0 || y < 0)
		map->addEntityRandom(mob);
	else
		map->addEntity(mob);

	return mob;
}

} /* namespace mob */
