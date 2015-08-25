/*
 * Entity.cpp
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#include "Entity.h"
#include "defines.h"
#include "DataService.h"
#include "Map.h"
#include <math.h>
#include "srv_proto.h"

Entity **Entity::oidTable = new Entity *[Viewable::maxOid];

Entity::Entity(int x, int y, Map *m, std::string name) :
    Viewable(x, y, m->getId()), hp(0), name(name), dir(DOWN), m(m)
{
    oidTable[getOid()] = this;
    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        effects[i] = 0;
    }
}

bool Entity::tryMove(char dir)
{
    if (dir > DIR_MAX)
        return false;
    if (!canMove())
        return false;
    Map *m = getMap();

    bool r = m->move(this, dir);
    if (r)
        this->dir = dir;

    return r;
}

Entity::~Entity()
{
    freeOid();

    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i])
            delete effects[i];
    }
}

void Entity::freeOid()
{
    oidTable[getOid()] = 0;
    //TODO consider releasing the oid maybe (so that it can be reused)
}

bool Entity::tryTurn(char dir)
{
    if (dir > DIR_MAX)
        return false;
    if (!canAct())
        return false;

    setDir(dir);
    m->forEachNearby(this, [&](Entity *e) {
        e->entityTurned(this, dir);
    });

    return true;
}

void Entity::setMap(Map *m)
{
    this->m = m;
    MapPoint::setMap(m->getId());
}

void Entity::struck(Entity *eatk, float dmg)
{
    int idmg;
    if (dmg > 1000000000.0)
        idmg = 1000000000;
    else
        idmg = (int) ceil(dmg);

    if (effects[StatusEffect::INVULNERABILITY]) {
        //cant strike me
        return;
    }

    if (effects[StatusEffect::SLEEP]) {
        idmg <<= 1;
        removeEffect(StatusEffect::SLEEP);
    }

    this->damage(eatk, idmg);

    m->forEachNearby(this, [&](Entity *e) {
        e->entityStruck(this, this->getDispHp(), 1);
    });
}

void Entity::damage(Entity *a, int amt)
{
    if (hp - amt > 0)
        hp -= amt;
    else
        hp = 0;
}

bool Entity::canSee(Viewable *v)
{
    return v == (Viewable *) this || v->getHideLevel() <= getSightLevel();
}

void Entity::appearanceChanged()
{
    m->forEachNearby(this, [&](Entity *e) {
        e->showViewable(this);
    });
}

Entity *Entity::getByOid(unsigned int oid)
{
    if (oid > maxOid)
        return 0;

    return oidTable[oid];
}

int Entity::getFasLevel()
{
    if (effects[StatusEffect::FAS])
        return effects[StatusEffect::FAS]->getVal();
    return 0;
}

StatusEffect *Entity::addStatusEffect(int seid)
{
    BaseEffect *be = BaseEffect::getById(seid);
    if (!be || effects[be->kind]) //lolo effects be kind pls
        return 0;

    StatusEffect *res = effects[be->kind] = new StatusEffect(be);
    if (be->kind == StatusEffect::CONCEAL) {
        m->forEachNearby(this, [&](Entity *e) {
            if (this != e && res->getVal() > e->getSightLevel())
            e->unshowViewable(this);
        });
        appearanceChanged();
    }

    return res;
}

short Entity::getSightLevel()
{
    if (effects[StatusEffect::SIGHT])
        return effects[StatusEffect::SIGHT]->getVal();
    else
        return 0;
}

short Entity::getHideLevel()
{
    if (effects[StatusEffect::CONCEAL])
        return effects[StatusEffect::CONCEAL]->getVal();
    else
        return 0;
}

void Entity::makeSound(unsigned char sound)
{
    getMap()->forEachNearby(this, [&](Entity *e) {
        //Server::playSound(c->getSession(), sound);
        e->playSound(sound);
    });
}

/**
 * Update this entity's status.
 *
 * Decreases remaining effect durations for entities. Overloading
 * implementations should call this method in their base class to ensure that
 * all time-related processing is completed.
 * @param[in] deferred A vector of actions to be taken some time after
 * the entity's present map has finished ticking.
 * @return true if this entity should be removed from its containing map, false
 * otherwise. The default implementation is guaranteed to return false
 */
bool Entity::tick(std::vector<std::function<void()> > &deferred)
{
    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i] && effects[i]->tick(this) <= 0) {
            removeEffect((StatusEffect::Kind) i);
        }
    }

    return false;
}

void Entity::playEffect(unsigned short effectId, int duration)
{
    m->forEachNearby(this, [&](Entity *e) {
        e->showEffect(this, effectId, duration);
    });
}

float Entity::modDmg()
{
    float r = 1.0;
    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i])
            r *= effects[i]->modDmg();
    }
    return r;
}

void Entity::doAction(char action, short duration)
{
    m->forEachNearby(this, [&](Entity *e) {
        e->showAction(this, action, duration);
    });
}

bool Entity::canAct()
{
    return !(effects[StatusEffect::SLEEP] || effects[StatusEffect::FREEZE]
        || effects[StatusEffect::DOOM]);
}

bool Entity::canMove()
{
    return canAct() && !(effects[StatusEffect::STUN]);
}

bool Entity::removeEffect(StatusEffect::Kind kind)
{
    if (!effects[kind])
        return false;

    Stats stchg;
    effects[kind]->getChange(stchg);
    stats -= stchg;

    delete effects[kind];
    effects[kind] = 0;

    if (kind == StatusEffect::CONCEAL)
        appearanceChanged();

    return true;
}

bool Entity::removeEffectId(int seid)
{
    BaseEffect *be = BaseEffect::getById(seid);
    assert(be);
    if (be) {
        if (effects[be->kind] && effects[be->kind]->getId() == seid) {
            return removeEffect((StatusEffect::Kind) be->kind);
        }
    }
    return false;
}

StatusEffect *Entity::addStatusEffect(int seid, int dur)
{
    BaseEffect *be = BaseEffect::getById(seid);
    if (!be || effects[be->kind]) //lolo effects be kind pls
        return 0;

    return effects[be->kind] = new StatusEffect(be, dur);
}

void Entity::heal(float amt, bool show)
{
    //TODO overflow test
    int iamt = (int) amt;
    if (hp + iamt > (int) getMaxHp())
        hp = getMaxHp();
    else if (hp + iamt < 0)
        hp = 0;
    else
        hp += iamt;

    if (show) {
        getMap()->forEachNearby(this, [&](Entity *e) {
            e->entityStruck(this, getDispHp(), 0xff);
        });
    }
}
