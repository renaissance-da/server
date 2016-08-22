/*
 * MobAI.cpp
 *
 *  Created on: 2013-06-13
 *      Author: per
 */

#include "MobAI.h"
#include "Mob.h"
#include <algorithm>
#include "Character.h"
#include "Combat.h"
#include "random_engines.h"

MobAI::MobAI(Mob *mob):
mob(mob)
{

}

MobAI::~MobAI()
{

}

bool MobAI::approach(MapPoint *p)
{
	short dx = p->getX() - mob->getX();
	short dy = p->getY() - mob->getY();

	char dirPref[4];
	short magX = dx >= 0 ? dx : -dx;
	short magY = dy >= 0 ? dy : -dy;
	if (dx < 0) {
		magX = -dx;
		dx = -1;
	}
	else {
		magX = dx;
		dx = 1;
	}
	if (dy < 0) {
		magY = -dy;
		dy = -1;
	}
	else {
		magY = dy;
		dy = 1;
	}

	if (magX > magY) {
		dirPref[0] = 2 - dx;
		dirPref[1] = 1 + dy;
		dirPref[2] = 1 - dy;
		dirPref[3] = 2 + dx;
	}
	else {
		dirPref[0] = 1 + dy;
		dirPref[1] = 2 - dx;
		dirPref[2] = 2 + dx;
		dirPref[3] = 1 - dy;
	}
	for (int i = 0; i < 4; i++) {
		if (mob->tryMove(dirPref[i])) {
			return true;
		}
	}

	return false;
}

bool matchesClassifier(Skill &sk, MobAI::SkillClassifier type)
{
	switch (type) {
	case MobAI::AOE:
		switch (sk.getTargetType()) {
		case TARGET_SELF_LARGE:
		case TARGET_SELF_MED:
		case TARGET_SELF_SMALL:
		case TARGET_CIRCLE:
		case TARGET_LARGE:
		case TARGET_MED:
		case TARGET_SMALL:
		case TARGET_ALL:
			return true;
		default:
			return false;
		}
		/* no break */
	case MobAI::SINGLE:
		switch (sk.getTargetType()) {
		case TARGET_TARGET:
			//TODO could apply to melee range or line skills
			return true;
		default:
			return false;
		}
		/* no break */
	case MobAI::MELEE:
		switch (sk.getTargetType()) {
		case TARGET_TARGET:
		case TARGET_FRONT:
		case TARGET_FIRST_LINE:
		case TARGET_LINE_LARGE:
		case TARGET_LINE_SMALL:
		case TARGET_CIRCLE:
		case TARGET_SELF_SMALL:
			return true;
		default:
			return false;
		}
		/* no break */
	case MobAI::SEE_HIDE:
		if (sk.getBase()->buff &&
				sk.getBase()->buff->kind == StatusEffect::SIGHT)
			return true;
		break;
	case MobAI::ANY:
		return true;
	}

	return false;
}

bool MobAI::useRandomSkill(Entity *t, double rate, SkillClassifier type)
{
	using std::vector;
	vector<unsigned int> avail;
	std::uniform_real_distribution<double> st_uniform_dist;
	for (unsigned int i = 0; i < mob->skills.size(); i++) {
		if (!mob->skills[i].first.onCd() && matchesClassifier(mob->skills[i].first, type))
			avail.push_back(i);
	}

	double nextRate = 0;
	//TODO could shuffle the available skills...
	for (unsigned ix : avail) {
		nextRate = mob->skills[ix].second * 0.01 * rate / (1.0 - nextRate);
		if (nextRate >= 1.0 ||
		    st_uniform_dist(generator()) <= nextRate) {
		    Combat::useSkill(mob, &mob->skills[ix].first, t);
		    mob->skills[ix].first.used(false, false);
		    return true; //The skill may have failed, but it was used
		}
	}
	return false; //failed
}

void MobAI::struck(Entity *atk, int idmg)
{
	for (Attacker &a : atkList) {
		if (a.who == atk->getOid()) {
			a.totalDamage += idmg;
			return;
		}
	}

	Attacker newAtk;
	newAtk.who = atk->getOid();
	newAtk.totalDamage = idmg;
	atkList.push_back(newAtk);
}

/**
 * \brief Notify this AI that one of the minions it created has died.
 *
 * Use to notify this AI that one of its minions has died. This updates the
 * AI's available minions list.
 * \param[in] m The minion who has died (but has not been deallocated).
 */
void MobAI::mobDied(Mob *m)
{
	for (auto it = minionList.begin(); it != minionList.end(); it++) {
		if (*it == m) {
			minionList.erase(it);
			return;
		}
	}
}

/**
 * \brief Add a minion to this AI's minions list.
 *
 * Use to keep track of an additional minion. This AI can use the minions list
 * to check the status of its minions, or even give them instructions.
 * \param[in] m The minion to add.
 */
void MobAI::addMinion(Mob *m)
{
	minionList.push_back(m);
}

/**
 * \brief Select a target from the nearby entities.
 *
 * Selects the preferred target from a list of valid targets.
 * Preferred generally means the one who's done the most damage to this,
 * but status effects and ac can affect this.
 * \param[in] near A list of nearby entities
 * \param[in] aggressive True if this mob will attack players preemptively
 * \return The preferred target of this AI
 */
Entity *MobAI::selectTarget(std::vector<Entity *> &nearby, bool aggressive)
{
	if (nearby.empty())
		return 0;

	Entity *secondary = 0;
	for (Attacker &a : atkList) {
		for (Entity *e : nearby) {
			if (a.who == e->getOid()) {
				//Check if this target is good - if its a player it must be alive and well
				if (e->getType() == Entity::E_CHARACTER) {
					Character *c = (Character *)e;
					if (c->isDead() || c->hasEffect(StatusEffect::DOOM)) {
						continue;
					}
				}

				//If the target is using hide, this becomes a low priority choice
				if (!mob->canSee(e) && mob->dist(e) > 1) {
					secondary = secondary ? secondary : e;
					continue;
				}

				//other considerations?

				return e;
			}
		}
	}

	//found no one, if hostile grab worst ac character
	if (aggressive) {
		short worstAc = 0x8000; //negative
		for (Entity *e : nearby) {
			if (e->getType() == Entity::E_CHARACTER) {
				Character *c = (Character *)e;
				if (c->getStats()->getAc() > worstAc && !c->isDead() &&
						!c->hasEffect(StatusEffect::DOOM) &&
						mob->canSee(c)) {
					worstAc = e->getStats()->getAc();
					secondary = e;
				}
			}
		}
	}
	return secondary;
}

/**
 * \brief Sort this AI's target list in order of preference to attack
 *
 * Reorder this AI's target list in order of preference to attack.
 * Without calling this, an AI should only change target if their target
 * becomes unavailable or a more preferred one becomes available again.
 */
void MobAI::reorderTargets()
{
	//Could probably reduce number of passes if it becomes important

	struct ordering {
		bool operator() (Attacker a, Attacker b) { return a.totalDamage > b.totalDamage; }
	} obj;

	//TODO purge 0 damage?
	/*for (auto it = atkList.begin(); it != atkList.end();) {
		if (!it->totalDamage)
			it = atkList.erase(it);
		else
			++it;
	}*/

	std::sort(atkList.begin(), atkList.end(), obj);

	for (Attacker &a : atkList) {
		a.totalDamage = 0;
	}
}

/**
 * \brief Turns and attacks a target
 *
 * Turns and attacks a target. There is no limit on the range of the
 * attack, so this should usually only be called when the mob is
 * adjacent to the target.
 * \param[in,out] t The target to attack
 */
void MobAI::attack(Entity *t)
{
	short dx = t->getX() - mob->getX();
	short dy = t->getY() - mob->getY();

	if (dx == 0)
		mob->tryTurn(dy < 0 ? 0 : 2);
	else
		mob->tryTurn(dx < 0 ? 3 : 1);
	Combat::useSkill(mob, &(mob->mobAssail), t);
}
