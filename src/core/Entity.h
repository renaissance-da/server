/*
 * Entity.h
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "IDataStream.h"
#include "MapPoint.h"
#include "Stats.h"
#include "element.h"
#include "StatusEffect.h"
#include <list>
#include <atomic>
#include <assert.h>
#include <string>
#include "Viewable.h"
#include "Door.h"
#include <functional>

class Map;

class Entity: public Viewable
{
public:
    enum EntityType
    {
        E_CHARACTER = 0,
        E_MOB = 1,
        E_PORTAL = 2,
        E_NPC = 3,
        E_MAX = 4,
        E_UNDEFINED
    };

    Entity(int x, int y, Map *m, std::string name);
    virtual ~Entity();

    //Map interacting functions
    bool tryMove(char dir);
    bool tryTurn(char dir);
    bool canAct();
    bool canMove();

    //Basic status functions
    unsigned int getHp()
    {
        return hp;
    }

    const std::string &getName()
    {
        return name;
    }
    void struck(Entity *eatk, float dmg);
    virtual void heal(float amt, bool show = true);
    virtual void damage(Entity *a, int amt);

    char getDir()
    {
        return dir;
    }
    void setDir(char dir)
    {
        this->dir = dir;
    }

    virtual EntityType getType()
    {
        return E_UNDEFINED;
    }
    virtual void gainExp(unsigned int exp)
    {
        return;
    }
    virtual const Stats *getStats()
    {
        return STATS_INVULNERABLE ;
    }
    virtual int getLevel()
    {
        return 0;
    }
    virtual unsigned short getDispHp()
    {
        return 100;
    }
    virtual int moveType()
    {
        return 0;
    }
    virtual void gearStruck()
    {
    }
    virtual void weaponStruck()
    {
    }
    int getFasLevel();
    virtual Element getAtkEle()
    {
        return None;
    }
    virtual Element getDefEle()
    {
        return None;
    }
    virtual unsigned int getMaxHp()
    {
        return 0;
    }
    virtual StatusEffect *addStatusEffect(int seid);
    virtual StatusEffect *addStatusEffect(int seid, int dur);
	bool doomed()
    {
        return effects[StatusEffect::DOOM] != nullptr;
    }
    bool hasEffect(StatusEffect::Kind k)
    {
        assert(k < StatusEffect::SE_KINDS);
        return effects[k] != nullptr;
    }
    virtual bool isSmall()
    {
        return true;
    }

    virtual unsigned char getAtkAnim()
    {
        return 1;
    }
    virtual int getMp()
    {
        return 0x7FFFFFFF;
    }
    virtual void addMp(int mp)
    {
    }
    virtual bool tick(std::vector<std::function<void()> > &deferred);
    virtual bool removeEffect(StatusEffect::Kind kind);bool removeEffectId(
        int seid);
    void playEffect(unsigned short effectId, int duration);
    void makeSound(unsigned char sound);
    float modDmg();
    void freeOid();
    short getSightLevel();
    virtual short getHideLevel() override;
    bool canSee(Viewable *v);

    void appearanceChanged();
    void doAction(char action, short duration);

    //Observation
    virtual void talked(Entity *who, std::string msg, unsigned char channel = 0)
    {
    }
    virtual void showViewable(Viewable *who)
    {
    }
    virtual void unshowViewable(Viewable *who)
    {
    }
    virtual void showViewables(const std::vector<Viewable *> &who)
    {
    }
    virtual void showAction(Viewable *who, char action, short duration)
    {
    }
    virtual void entityStruck(Entity *who, unsigned short dispHp,
        unsigned char sound)
    {
    }
    virtual void showEffect(Viewable *who, unsigned short effectId,
        int duration)
    {
    }
    virtual void playSound(unsigned char sound)
    {
    }
    virtual void entityTurned(Entity *who, char direction)
    {
    }
    virtual void entityMoved(Entity *who, char direction)
    {
    }
    virtual void showDoor(Door *door)
    {
    }
    virtual void showDoors(const std::vector<Door *> &doors)
    {
    }
    virtual void sendMessage(const char *text, unsigned char channel = 3)
    {
    }

    static Entity *getByOid(unsigned int oid);
    Map *getMap()
    {
        return m;
    }
    void setMap(Map *m);

protected:
    int hp;
    Stats stats;
    StatusEffect *effects[StatusEffect::SE_KINDS];
    std::string name;

private:
    char dir;
    Map *m;

    static Entity **oidTable;
};

#endif /* ENTITY_H_ */
