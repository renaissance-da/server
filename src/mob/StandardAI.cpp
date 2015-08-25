/*
 * StandardAI.cpp
 *
 *  Created on: 2013-06-13
 *      Author: per
 */

#include "StandardAI.h"
#include "Combat.h"
#include "random.h"

StandardAI::StandardAI(Mob *mob, int subMode):
MobAI(mob),
rethinkDelay(8*TICKS),
actDelay(5*TICKS),
clearDelay(10*TICKS),
maxConfuses(2),
angry(false),
sleepy(true)
{
	hostile = subMode == 1;
}

void StandardAI::runAI()
{
	//standard ai sleeps when no players nearby
	if (!mob->getMap()->hasPlayers()) {
		if (--clearDelay == 0) {
			//Go to sleep
			actDelay = 2 * TICKS;
			rethinkDelay = 4 * TICKS;
			atkList.clear();
			sleepy = true;
		}
		return;
	}

	clearDelay = 10 * TICKS;

	//Think about new targets?
	if (--rethinkDelay < 0) {
		reorderTargets();
		rethinkDelay = 8 * TICKS;
		sleepy = false;
	}

	//Standard AI mobs treat stun as sleep
	if (mob->hasEffect(StatusEffect::SLEEP)) {
		angry = true;
		return;
	}
	else if (mob->hasEffect(StatusEffect::STUN)) {
		return;
	}

	//Still delayed from last action?
	if (actDelay >= 0) {
		--actDelay;
		return;
	}

	std::vector<Entity *> near;
	mob->getMap()->getNearbyEntities(mob, near);
	Entity *t = sleepy ? 0 : selectTarget(near, hostile);

	if (t) {
		if (!mob->canSee(t) && mob->dist(t) > 1) {
			//Can cast see hide?
			if (!mob->hasEffect(StatusEffect::SIGHT)) {
				if (useRandomSkill(t, 100.0, SkillClassifier::SEE_HIDE)) {
					actDelay = stt(1);
					return;
				}
			}
			//Try to aoe
			if (useRandomSkill(t, 5.0, SkillClassifier::AOE)) {
				actDelay = stt(1);
				return;
			}
			//Move randomly
			mob->tryMove(random() % 4);
			actDelay = stt(1);
			return;
		}

		//need to approach?
		short dist = t->dist(mob);
		if (dist > 1) {
			//Blind mobs dont walk
			if (!mob->hasEffect(StatusEffect::BLIND)) {
				approach(t);
				actDelay = stt(1);
			}
			else {
				angry = true;
				rethinkDelay = rethinkDelay < stt(3) ? rethinkDelay : stt(3);
				return; //wont use spell
			}
		}
		else {
			//turn and attack
			attack(t);
			actDelay = stt(1);
			angry = false; //prevents awakened mob from using magic on an adjacent target
		}
		if (angry)
			angry = !useRandomSkill(t, 100.0);
		else
			useRandomSkill(t, dist > 1 ? 1.0 : 0.5);
	}

	else {
		int choice = random() % 6;
		switch (choice) {
		case 0:
		case 1:
		case 2:
			mob->tryMove(random()%4);
			actDelay = stt(1);
			break;
		case 3:
		case 4:
			mob->tryTurn(random()%4);
			actDelay = stt(2);
			break;
		case 5:
			actDelay = 1;
			break;
		}
	}
}

void StandardAI::confuse()
{
	if (maxConfuses > 0) {
		--maxConfuses;
		actDelay = stt(3);
		atkList.clear();
	}
}

void StandardAI::taunt(Entity *e)
{
	int max = 1;
	for (Attacker &a : atkList) {
		if (a.totalDamage > max)
			max = a.totalDamage + 1;
	}

	struck(e, max);
	reorderTargets();
	rethinkDelay = 8 * TICKS;
}

StandardAI::~StandardAI() {
}

