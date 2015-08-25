/*
 * Mob.h
 *
 *  Created on: 2012-12-15
 *      Author: per
 */

#ifndef MOB_H_
#define MOB_H_

#include "Entity.h"
#include "IDataStream.h"
#include "Stats.h"
#include "Skill.h"
#include <vector>
#include "MobWatcher.h"
#include "defines.h"
#include "Item.h"
#include "element.h"
#include <string>

class MobAI;

class Mob: public Entity {
public:
	Mob(unsigned short x, unsigned short y, Map *map, unsigned short apr, int id, unsigned int hp=50,
			unsigned int min=4, unsigned short max=6, unsigned int exp=100, std::string name="Bob Marley",
			unsigned short mode=0, unsigned short lev=0, Element atk=None, Element def=None,
			short ac=100, short power=0, short mr=0, short dex=0, bool small=true, unsigned short regen=0,
			short submode=2);
	virtual ~Mob();

	bool tick(std::vector<std::function<void ()> > &deferred) override;
	void getViewedBlock(IDataStream *dp) override;
	virtual EntityType getType() override { return E_MOB; }
	void damage(Entity *a, int dmg) override;
	virtual const Stats *getStats() override { return &stats; }
	int getLevel() override { return level; }
	unsigned short getDispHp() override;
	void spawnedBy(MobWatcher *who) { spawner = who; }
	void dropItems(Entity *owner);
	int moveType() { return MOVE_NO_PORTALS | MOVE_SOLID; }
	void giveGold(int amt) { gold += amt; }
	void giveItem(Item *itm) { items.push_back(itm); }
	unsigned int getMaxHp() { return stats.getHp(); }
	StatusEffect *addStatusEffect(int seid);
	StatusEffect *addStatusEffect(int seid, int dur);
	void setSkills(std::vector<std::pair<int, short> > &skillList);
	//void useSkills(Entity *t, unsigned short rateDenom);
	unsigned int getExp() { return exp; }
	void confuse();
	void taunt(Entity *e);

	Element getAtkEle() { return a; }
	Element getDefEle();
	bool isSmall() { return small; }

	friend class MobAI;
	MobAI *getAI() { return ai; }

private:

	unsigned short appearance, mode, actDelay, level, regenTick, regen;
	unsigned int exp, mobId;
	int lastAttacker, gold;
	Skill mobAssail;
	MobWatcher *spawner;
	Element a, d;
	bool small;

	std::vector<Item *> items;
	std::vector<std::pair<Skill, short> > skills;
	MobAI *ai;
};

#endif /* MOB_H_ */
