/*
 * Spawner.h
 *
 *  Created on: 2013-01-01
 *      Author: per
 */

#ifndef SPAWNER_H_
#define SPAWNER_H_

#include <list>
#include "BaseItem.h"
#include "element.h"
#include "MobWatcher.h"

class Map;

class Spawner : public MobWatcher {
public:
	struct Drop {
		BaseItem *bi;
		unsigned short rate;
		short mod;
	};

	Spawner(int mobid, Map *m, short qty, int frequency);
	Spawner(const Spawner *other, Map *m);
	~Spawner();

	void tick();
	void forceAll();
	void mobDied(Mob *m) override;
	void spawnMob();
	void addMobSkill(int skillId, short rate);

private:
	int frequency, mobId;
	unsigned tickCount;
	short qty;
	Map *m;

	std::list<unsigned> todos;
};

#endif /* SPAWNER_H_ */
