/*
 * Spawner.cpp
 *
 *  Created on: 2013-01-01
 *      Author: per
 */

#include "Spawner.h"
#include "random.h"
#include "Mob.h"
#include "Map.h"
#include "Equipment.h"
#include "MobInfo.h"

Spawner::Spawner(
		int mobid,
		Map *m,
		short qty,
		int frequency):
frequency(frequency),
mobId(mobid),
tickCount(0),
qty(qty),
m(m)
{
	for (unsigned short i = 0; i < qty; i++) {
		//spawnMob();
		todos.push_back(tickCount);
	}
}

Spawner::Spawner(const Spawner *other, Map *m):
		frequency(other->frequency),
		mobId(other->mobId),
		tickCount(0),
		qty(other->qty),
		m(m)
{
	for (unsigned short i = 0; i < qty; i++) {
		todos.push_back(tickCount);
	}
}

Spawner::~Spawner()
{

}

void Spawner::forceAll()
{
	for (int i = 0; i < todos.size(); i++) {
		spawnMob();
	}
	todos.clear();
}

void Spawner::tick()
{
	tickCount = (tickCount + 1) % frequency;
	auto it = todos.begin();
	while (it != todos.end() && *it == tickCount) {
		spawnMob();
		it = todos.erase(it);
	}
}

void Spawner::spawnMob()
{
	Mob *mob = mob::MobInfo::spawnById(mobId, -1, -1, m);
	mob->spawnedBy(this);
}

void Spawner::mobDied(Mob *m)
{
	todos.push_back(tickCount);
}
