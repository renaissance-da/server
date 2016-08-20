/*
 * MobAI.h
 *
 *  Created on: 2013-06-13
 *      Author: per
 */

#ifndef MOBAI_H_
#define MOBAI_H_

#include "Mob.h"
#include <vector>
#include "defines.h"
#include "MobWatcher.h"
#include <list>

class MobAI : public MobWatcher {
public:
	enum SkillClassifier {
		AOE,
		SINGLE,
		MELEE,
		SEE_HIDE,
		ANY
	};

	MobAI(Mob *mob);
	virtual ~MobAI();

	virtual void runAI() = 0;
	virtual void struck(Entity *atk, int idmg);
	virtual void confuse() {}
	virtual void taunt(Entity *e) {}

	//helpers
	bool approach(MapPoint *p);
	bool useRandomSkill(Entity *t, double rate, SkillClassifier type = ANY);
	Entity *selectTarget(std::vector<Entity *> &nearby, bool aggressive);
	void reorderTargets();
	int stt(int sec) { return sec * TICKS - 1; }
	void attack(Entity *t);

	//minions
	void mobDied(Mob *m) override;
	void addMinion(Mob *m);
	const std::list<Mob *> &getMinions() { return minionList; }

protected:
	Mob *mob;
	struct Attacker{
		//Entity *who;
		int who;
		long long totalDamage;
	};
	std::vector<Attacker> atkList;

private:
	std::list<Mob *> minionList;
};

#endif /* MOBAI_H_ */
