/*
 * Mob.cpp
 *
 *  Created on: 2012-12-15
 *      Author: per
 */

#include "Mob.h"
#include "random.h"
#include "srv_proto.h"
#include "DataService.h"
#include "Character.h"
#include <algorithm>
#include "Combat.h"
#include "ai_broker.h"

Mob::Mob(unsigned short x, unsigned short y, Map *map, unsigned short apr, int id,
		unsigned int hp, unsigned int min, unsigned short max, unsigned int exp, std::string name,
		unsigned short mode, unsigned short lev, Element atk, Element def, short ac, short power,
		short mr, short dex, bool small, unsigned short regen, short submode):
Entity(x, y, map, name),
appearance(apr),
mode(mode),
actDelay(0),
level(lev),
regenTick(0),
regen(regen),
exp(exp),
mobId(id),
lastAttacker(0),
gold(0),
mobAssail(3, (Path)0, 100),
spawner(0),
a(atk),
d(def),
small(small)
{
	this->hp = hp;
	stats.setMaxHp(hp);
	stats.setMinAtk(min);
	stats.setMaxAtk(max);
	stats.setAc(ac);
	stats.setInt(power);
	stats.setDex(dex);
	stats.setMr(mr);

	ai = mob::getAI(mode, submode, this);
}

Mob::~Mob() {
	delete ai;
}


Element Mob::getDefEle()
{
	if (effects[StatusEffect::ELEMENT])
		return (Element) effects[StatusEffect::ELEMENT]->getVal();

	return d;
}

bool Mob::tick(std::vector<std::function<void ()> > &deferred)
{
	//tick effects etc
	Entity::tick(deferred);
	//tick skills
	for (auto it = skills.begin(); it != skills.end(); it++) {
		it->first.decCd();
	}

	if (getHp() > 0) {
		if (++regenTick >= TICKS) {
			regenTick = 0;
			int newhp = hp + (stats.getHp() * regen) / 100;
			hp = newhp > stats.getHp() ? stats.getHp() : newhp;
		}
		ai->runAI();
		return false;
	}

	Map *m = getMap();
	Entity *e = 0;
	if (lastAttacker && (e = m->getNearByOid(this, lastAttacker)))
		DataService::getService()->groupDistributeExp(exp, level, mobId, e);

	if (spawner)
		spawner->mobDied(this);

	dropItems(e);

	deferred.push_back([=]() {
		delete this;
	});

	return true;
}

StatusEffect *Mob::addStatusEffect(int seid)
{
	StatusEffect *se = Entity::addStatusEffect(seid);
	if (!se)
		return 0;
	Stats stchg;
	se->getChange(stchg);
	stats += stchg;
	return se;
}

StatusEffect *Mob::addStatusEffect(int seid, int dur)
{
	StatusEffect *se = Entity::addStatusEffect(seid, dur);
	if (!se)
		return 0;
	Stats stchg;
	se->getChange(stchg);
	stats += stchg;
	return se;
}


void Mob::dropItems(Entity *owner)
{
	Map *m = getMap();
	std::vector<unsigned int> protect;
	if (owner && owner->getType() == Entity::E_CHARACTER)
	{
		Character *c = (Character *)owner;
		std::list<Character *> *g = c->getGroup();
		if (g) {
			for (Character *member : *g) {
				protect.push_back(member->getId());
			}
		}
		else
			protect.push_back(c->getId());
	}

	for (auto it = items.begin(); it != items.end(); it = items.erase(it))
		if (!m->putItem((*it), getX(), getY(), &protect, 120))
			delete (*it);

	m->putGold(gold, getX(), getY(), &protect, 120);
}

void Mob::getViewedBlock(IDataStream *dp)
{
	dp->appendShort(getX());
	dp->appendShort(getY());
	dp->appendInt(getOid());
	dp->appendShort(appearance | 0x4000);
	dp->appendInt(0);
	dp->appendByte(getDir());
	dp->appendByte(0);
	dp->appendByte(0);//type=0 mob
}

unsigned short Mob::getDispHp()
{
	//for %'s, use return hp * 100 / stats.getHp();
	//server further constrains to measurements from 0 to 5
	return (hp * 100 / stats.getHp() / 20 * 20);
}

void Mob::damage(Entity *a, int dmg)
{
	if (a && a != this) {
		lastAttacker = a->getOid();
		ai->struck(a, (int)dmg);
	}

	Entity::damage(a, dmg);
}

void Mob::setSkills(std::vector<std::pair<int, short int> > &skillList)
{
	std::for_each(skillList.begin(), skillList.end(), [&](std::pair<int, short int> p) {
		skills.push_back(std::pair<Skill, int>(Skill(p.first, Peasant, 100), p.second));
	});
}

void Mob::confuse()
{
	ai->confuse();
}

void Mob::taunt(Entity *e)
{
	ai->taunt(e);
}

