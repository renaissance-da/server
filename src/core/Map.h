/*
 * Map.h
 *
 *  Created on: 2012-06-16
 *      Author: per
 */

#ifndef MAP_H_
#define MAP_H_

#include <vector>
#include "Entity.h"
#include "IDataStream.h"
#include <string>
#include "Portal.h"
#include "Door.h"
#include "Spawner.h"
#include "MapPoint.h"
#include "defines.h"
#include "GroundItem.h"
#include <list>
#include "Trigger.h"
#include <mutex>
#include <errno.h>
#include <functional>
#include "Timer.h"
#include <queue>
#include <chrono>

class Map
{
public:

    enum MapFlags
    {
        MF_PVP = 1,
        MF_NO_SKILLS = 2,
        MF_NO_SPELLS = 4,
        MF_NO_SCROLLS = 8,
        MF_INSTANCE = 16
    };

    Map(const char *name, unsigned short width, unsigned short height,
        unsigned short bgm, unsigned short crc, unsigned char *walls,
        unsigned short id, unsigned char *file, unsigned flags);
    Map(Map *other, unsigned short id);
    virtual ~Map();
    void tick(std::vector<std::function<void()> > &deferred);

    bool isWall(unsigned short x, unsigned short y, int flags);bool tryFindFree(
        Entity *e, unsigned short rad);
    void removeEntity(Entity *e);
    void lock();
    void unlock();

    //TODO remove this function
    void addSpawner(Spawner *s);

    bool putItem(Item *item, unsigned short x, unsigned short y,
        std::vector<unsigned int> *protection = 0, 
		std::chrono::seconds protectionTime = std::chrono::seconds(0));

    bool putGold(unsigned int amt, unsigned short x, unsigned short y,
        std::vector<unsigned int> *protection = 0, 
		std::chrono::seconds protectionTime = std::chrono::seconds(0));

    Portal *addPortal(unsigned short x, unsigned short y, unsigned short xDest,
        unsigned short yDest, unsigned short dst);

    void addEntity(Entity *m, unsigned short rad = 0);
    void addEntityRandom(Entity *m);

    //MapPoint accessors
    Portal *getPortal(unsigned short x, unsigned short y);
    Door *getDoor(unsigned short x, unsigned short y);
    GroundItem *getItem(unsigned short x, unsigned short y,
        unsigned int looterId);

    template<typename E>
    void getItems(unsigned short x, unsigned short y, std::vector<E *> &itms);

    void getNearbyDoors(MapPoint *e, std::vector<Door *> &nearby);

    void getNearbyEntities(MapPoint *e, std::vector<Entity *> &nearby);

    void getNearbyViewables(MapPoint *e, std::vector<Viewable *> &nearby,
        bool includeEntities = true);
    void getThresholdDoors(MapPoint *e, std::vector<Door *> &nearby);
    Entity *getNearByOid(MapPoint *e, unsigned int oid, int dist = NEARBY);
    Entity *getEnt(MapPoint *e);
    void getEnts(const std::vector<MapPoint> &area,
        std::vector<Entity *> &targets, Entity *exclusion = 0);
	bool hasPlayers()
    {
        return nPlayers != 0;
    }

    void addTrigger(Trigger *t);
    void addTimer(Timer *t);
    void getTriggers(std::vector<Trigger *> &triggers, unsigned short x,
        unsigned short y);

    void getZoneData(IDataStream *s);
    char getBgm();
    unsigned short getId()
    {
        return id;
    }
    virtual unsigned short getDispId()
    {
        return getId();
    }

    bool move(Entity *e, char dir);bool mapJump(Entity *e, unsigned short x,
        unsigned short y);
    void talked(Entity *e, std::string text);

    void forEachNearby(MapPoint *e, std::function<void(Entity *)> fn);
    const unsigned char *getFile()
    {
        return file;
    }
    unsigned char getWidth()
    {
        return width;
    }
    unsigned char getHeight()
    {
        return height;
    }
    unsigned short getCrc()
    {
        return crc;
    }

    //Properties
    bool pvpOn()
    {
        return flags & MF_PVP;
    }
    bool noSkills()
    {
        return (flags & MF_NO_SKILLS) == MF_NO_SKILLS;
    }
    bool noSecrets()
    {
        return (flags & MF_NO_SPELLS) == MF_NO_SPELLS;
    }
    bool noScrolls()
    {
        return (flags & MF_NO_SCROLLS) == MF_NO_SCROLLS;
    }
    bool isInstance()
    {
        return (flags & MF_INSTANCE) == MF_INSTANCE;
    }

    const char *getName()
    {
        return mapName.c_str();
    }

    void togglePvp();

protected:

    int width, height;
    unsigned char *walls;
    unsigned char *file;
    char bgm;
    std::string mapName;

    unsigned short crc, id;

    //TODO need to come up with a sorted way of storing entities in general
    //could use a row->column pair of maps
    std::vector<Portal> ports;
    std::vector<Door> doors;
    std::list<Entity *> entities;
    //std::vector<Entity *> *entityTable;
    std::vector<Spawner *> spawners;
    std::list<GroundItem *> items;
    std::vector<Trigger *> triggers;
    std::priority_queue<Timer *, std::vector<Timer *>, Timer::TimerCompare> timers;

    //TODO decide if this is permanent
    int nPlayers;

    std::mutex mapLock;

    static std::map<unsigned short, bool> doorList;
    unsigned flags;

    template<typename T>
    void partitionMoveResults(const std::vector<T *> &prev,
        const std::vector<T *> &next, std::vector<T *> &gone,
        std::vector<T *> &remain, std::vector<T *> &appear);
};

template<typename E>
void Map::getItems(unsigned short x, unsigned short y, std::vector<E *> &nearby)
{
    for (GroundItem *ge : items) {
        if (ge->getX() == x && ge->getY() == y)
            nearby.push_back(ge);
    }
}

#endif /* MAP_H_ */
