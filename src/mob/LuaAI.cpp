/*
 * LuaAI.cpp
 *
 *  Created on: Feb 15, 2014
 *      Author: per
 */

#include <LuaAI.h>
#include "Map.h"
#include "ai_bindings.h"
#include "Character.h"
#include <algorithm>

LuaAI::LuaAI(int submode, Mob *m):
MobAI(m)
{
	L = script::newLuaAI(submode, m);
}

LuaAI::~LuaAI()
{
	script::freeLuaAI(L);
}

void LuaAI::runAI()
{
	Map *m = mob->getMap();

	std::vector<Entity *> nearby;
	m->getNearbyEntities(mob, nearby);

	std::vector<Entity *> targets;

	//Sort attackers
	struct ordering {
		bool operator() (Attacker a, Attacker b) {
			return a.totalDamage > b.totalDamage;
		}
	} ord;
	std::sort(atkList.begin(), atkList.end(), ord);

	//Filter targets
	for (Attacker &a : atkList) {
		for (Entity *e : nearby) {
			if (a.who == e->getOid()) {
				if (e->getType() == Entity::E_CHARACTER) {
					Character *c = (Character *)e;
					if (c->isDead() || c->hasEffect(StatusEffect::DOOM)) {
						break;
					}
				}
				targets.push_back(e);
				break;
			}
		}
	}

	script::act(L, nearby, targets);
}
