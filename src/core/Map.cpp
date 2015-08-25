/*
 * Map.cpp
 *
 *  Created on: 2012-06-16
 *      Author: per
 */

#include "Map.h"
#include "defines.h"
#include <algorithm>
#include <assert.h>
#include "random.h"
#include "LockSet.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

std::map<unsigned short, bool> Map::doorList {
	{2714, true}, {2198, true}, {2852, true}, {2924, true}, {2682, true},
	{2227, true}, {2291, true}, {2768, true}, {2876, true}, {2260, true},
	{2728, true}, {2874, true}, {2999, true}, {3158, true}, {3210, true},
	{2727, true}, {2904, true}, {2734, true}, {2945, true}, {3159, true},
	{2164, true}, {2777, true}, {3058, true}, {2000, true}, {2261, true},
	{2850, false}, {2292, true}, {2228, true}, {2293, true}, {2851, true},
	{2769, true}, {2694, true}, {2681, true}, {2952, true}, {3118, true},
	{2946, true}, {2695, true}, {2903, true}, {2674, true}, {2680, true},
	{3178, true}, {2436, true}, {2165, true}, {2761, true}, {2923, true},
	{2461, true}, {2951, true}, {2776, true}, {2673, false}, {2696, true},
	{2875, true}, {3000, true}, {2163, true}, {2196, false}, {2262, true},
	{2688, true}, {2675, true}, {2715, true}, {3059, true}, {3179, true},
	{2762, true}, {3211, true}, {1994, true}, {3119, true}, {2735, true},
	{3099, true}, {2687, true}, {2689, true}, {2197, true}, {2229, true},
	{3098, true}
};

Map::Map(Map *other, unsigned short id):
		Map(other->mapName.c_str(),
		other->width,
		other->height,
		other->bgm,
		other->crc,
		other->walls,
		id,
		other->file,
		other->flags)
{
	//copy all spawners
	for (Spawner *s : other->spawners) {
		Spawner *ns = new Spawner(s, this);
		spawners.push_back(ns);
	}
}

Map::Map(const char *name, unsigned short width, unsigned short height, unsigned short bgm, unsigned short crc, unsigned char *walls, unsigned short id, unsigned char *file, unsigned flags):
walls(walls),
file(file),
mapName(name),
crc(crc),
id(id),
nPlayers(0),
#ifndef NDEBUG
mapLock(PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP),
#else
mapLock(PTHREAD_MUTEX_INITIALIZER),
#endif
flags(flags)
{
	assert(width > 0 && width < 256);
	assert(height > 0 && height < 256);
	this->width = (unsigned char)width;
	this->height = (unsigned char)height;
	this->bgm = (unsigned char)bgm;

	//walls = new unsigned char[width * height];
	//readBlock(wallFd, (char *)walls, width * height);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//left facing doors are placed on the right side of each tile
			unsigned short lid, rid;
			//assumes little endian
			lid = *((unsigned short *)(file + 6*(i + j*width) + 2));
			rid = *((unsigned short *)(file + 6*(i + j*width) + 4));
			auto ld = doorList.find(lid);
			auto rd = doorList.find(rid);
			if (ld != doorList.end()) {
				doors.push_back(Door(i, j, this, 1));
				IS_WALL(i,j) = ld->second ? 0 : 1;
			}
			else if (rd != doorList.end()) {
				doors.push_back(Door(i, j, this, 0));
				IS_WALL(i,j) = rd->second ? 0 : 1;
			}
		}
	}
}

Map::~Map() {
	if (walls)
		delete[] walls;
	if (file)
		delete[] file;

	ports.clear();
	doors.clear();
	std::for_each(entities.begin(), entities.end(), [&](Entity *m) {
		assert(m->getType() != Entity::E_CHARACTER);
		delete m;
	});
	std::for_each(spawners.begin(), spawners.end(), [&](Spawner *s) {
		delete s;
	});
	std::for_each(items.begin(), items.end(), [&](GroundItem *i) {
		delete i->getItem(); delete i;
	});
	std::for_each(triggers.begin(), triggers.end(), [&](Trigger *t) {
		delete t;
	});
	while (!timers.empty()) {
		delete timers.top();
		timers.pop();
	}

	pthread_mutex_destroy(&mapLock);
}

void Map::togglePvp()
{
	flags ^= MF_PVP;
	if (flags & MF_PVP) {
		for (Entity *e : entities) {
			e->sendMessage("Warning! PVP mode is now on!");
		}
	}
	else {
		for (Entity *e : entities) {
			e->sendMessage("PVP mode is now off");
		}
	}
}

bool Map::isWall(unsigned short x, unsigned short y, int flags)
{
	if (x >= width || y >= height) {
		return true;
	}
	if (IS_WALL(x, y))
		return true;

	if (flags & MOVE_SOLID) {
		for (auto it = entities.begin(); it != entities.end(); it++)
			if ((*it)->getX() == x && (*it)->getY() == y)
				return true;
		for (auto it = doors.begin(); it != doors.end(); it++)
			if (it->getX() == x && it->getY() == y && it->isClosed())
				return true;
	}
	if (flags & MOVE_NO_PORTALS) {
		for (auto it = ports.begin(); it != ports.end(); it++)
			if (it->getX() == x && it->getY() == y)
				return true;
	}
	return false;
}

void Map::talked(Entity *e, std::string text)
{
	if (text.length() > 68)
		text = text.substr(0, 67);

	forEachNearby(e, [&](Entity *other) {
		other->talked(e, text);
	});
}

bool Map::tryFindFree(Entity *e, unsigned short rad)
{
	int flags = e->moveType() | MOVE_NO_PORTALS;

	if (!isWall(e->getX(), e->getY(), flags))
		return true;

	unsigned short x = e->getX();
	unsigned short y = e->getY();

	//Try to find an unobstructed tile
	for (unsigned short r = 1; r <= rad; r++) { //Current search radius
		for (unsigned short z = 0; z < r; z++) { //current search rotation
			//z=0 corresponds to the point r above the character
			//moves c.w. from there to the point directly right of character by r (less 1)
			if (!isWall(x+z, y-r+z, flags)) { //note, if these coords roll over they will always appear as walls
				//found a spot
				e->setX(x + z);
				e->setY(y - r + z);
				return true;
			}
		}
		for (unsigned short z = 0; z < r; z++) {
			//z=0 now corresponds to the point directly right of the character
			if (!isWall(x+r-z, y+z, flags)) {
				e->setX(x + r - z);
				e->setY(y + z);
				return true;
			}
		}
		for (unsigned short z = 0; z < r; z++) {
			//z=0 corresponds to directly below character
			if (!isWall(x-z, y+r-z, flags)) {
				e->setX(x - z);
				e->setY(y + r - z);
				return true;
			}
		}
		for (unsigned short z = 0; z < r; z++) {
			//z=0 corresponds to directly left of character
			if (!isWall(x-r+z, y-z, flags)) {
				e->setX(x - r + z);
				e->setY(y - z);
				return true;
			}
		}
	}
	return false;
}

void Map::getNearbyViewables(MapPoint *e, std::vector<Viewable *> &nearby, bool includeEntities)
{
	if (includeEntities) {
		std::for_each(entities.begin(), entities.end(), [&](Entity *m) {
			if (MH_DIST(e->getX(), m->getX(), e->getY(), m->getY()) <= NEARBY)
				nearby.push_back(m);
		});
	}

	std::for_each(items.begin(), items.end(), [&](GroundItem *i) {
		if (MH_DIST(e->getX(), i->getX(), e->getY(), i->getY()) <= NEARBY)
			nearby.push_back(i);
	});
}


Portal *Map::getPortal(unsigned short x, unsigned short y)
{
	for (unsigned int i = 0; i < ports.size(); i++) {
		if (ports[i].getX() == x && ports[i].getY() == y) {
			return &(ports[i]);
		}
	}
	return 0; //Nothing stepped on
}

void Map::addEntity(Entity *m, unsigned short rad)
{
	if (m->getType() == Entity::E_CHARACTER)
		++nPlayers;

	if (rad) {
		tryFindFree(m, rad);//If it fails, we still have to insert. However, we may later want to mark characters as stuck
	}

	std::vector<Viewable *> viewables;
	getNearbyViewables(m, viewables);
	m->showViewables(viewables);

	entities.push_back(m);

	forEachNearby(m, [&](Entity *e) {
		e->showViewable(m);
	});
}

void Map::addEntityRandom(Entity *m)
{
	for (int i = 0; i < 10; i++) {
		m->setX(random()%width);
		m->setY(random()%height);
		if (tryFindFree(m, 3))
			break;
	}

	//TODO better method?
	addEntity(m);
}

Portal *Map::addPortal(unsigned short x, unsigned short y, unsigned short xDest, unsigned short yDest, unsigned short dst)
{
	ports.push_back(Portal(x, y, xDest, yDest, dst));
	assert(x < width && y < height);
	IS_WALL(x,y) = 0;
	return &ports.back();
}

/**
 * Populates a list with the triggers on a given spot.
 * \param[out] triggers The list to which all of the triggers on the given spot
 * 		will be added. Triggers already in the list will remain there.
 * 		This list becomes invalid once the map's lock has been released.
 * \param[in] x The x coordinate of the spot to find the triggers on.
 * \param[in] y The y coordinate of the spot to find the triggers on.
 */
void Map::getTriggers(std::vector<Trigger *> &triggers, unsigned short x,
		unsigned short y)
{
	for (Trigger *t : this->triggers) {
		if (t->getX() == x && t->getY() == y)
			triggers.push_back(t);
	}
}

bool Map::putItem(Item *item, unsigned short x, unsigned short y, std::vector<unsigned int> *protection, unsigned int protectionTime)
{
	if (Map::isWall(x, y, MOVE_NO_PORTALS | MOVE_NO_DOORS))
		return false;

	GroundItem *newItem = new GroundItem(item, id, x, y, protectionTime, protection);
	items.push_back(newItem);
	forEachNearby(newItem, [&](Entity *e) {
		e->showViewable(newItem);
	});
	return true;
}

void Map::removeEntity(Entity *e)
{
	if (e->getType() == Entity::E_CHARACTER)
		--nPlayers;

	for (auto it = entities.begin(); it != entities.end(); it++) {
		if (*it == e) {
			entities.erase(it);
			break;
		}
	}

	forEachNearby(e, [&](Entity *o) {
		o->unshowViewable(e);
	});

}

Entity *Map::getNearByOid(MapPoint *e, unsigned int oid, int dist)
{
	Entity *r = Entity::getByOid(oid);
	if (r && (signed) e->dist(r) <= dist)
		return r;

	return 0;
}

/**
 * \brief Add a trigger to the map.
 *
 * Add a trigger to the map. The trigger will be activated when it is stepped
 * on.
 * \param[in] t The trigger to be added. The trigger will be destroyed when it
 * signals that is has expired or when the map is destroyed.
 */
void Map::addTrigger(Trigger *t)
{
	triggers.push_back(t);
}

/**
 * \brief Add a timer to the map.
 *
 * Add a timer to the map. The timer will be triggered when time() returns
 * a value less than the timer's activation time.
 * \param[in] t The timer to be added. Timers are destroyed after they are
 * activated, or when the map is destroyed.
 */
void Map::addTimer(Timer *t)
{
	timers.push(t);
}

void Map::lock()
{
	assert(LockSet::addLock(id));
	int r = pthread_mutex_lock(&mapLock);
	assert(!r);
}

void Map::unlock()
{
	assert(LockSet::removeLock(id));
	int r = pthread_mutex_unlock(&mapLock);
	assert(!r);
}

void Map::getZoneData(IDataStream *s)
{
	s->appendByte((char)width);
	s->appendByte((char)height);
	s->appendByte(0);
	s->appendShort(0);
	s->appendShort(crc);
	s->appendString(mapName.length(), mapName.c_str());
	s->appendByte(0);
}

char Map::getBgm()
{
	return bgm;
}

//PRE prev, next are sorted
template
<typename T>
void Map::partitionMoveResults(
		const std::vector<T *> &prev,
		const std::vector<T *> &next,
		std::vector<T *> &gone,
		std::vector<T *> &remain,
		std::vector<T *> &appear)
{
	auto iold = prev.begin(), inew = next.begin();
	while (iold != prev.end() && inew != next.end()) {
		if (*iold < *inew)
			gone.push_back(*(iold++));
		else if (*inew < *iold)
			appear.push_back(*(inew++));
		else {
			remain.push_back(*(iold++));
			++inew;
		}
	}

	while (iold != prev.end())
		gone.push_back(*(iold++));
	while (inew != next.end())
		appear.push_back(*(inew++));
}

bool Map::move(Entity *e, char dir)
{
	int xDest = e->getX();
	int yDest = e->getY();
	switch (dir) {
	case UP:
		yDest -= 1;
		break;
	case DOWN:
		yDest += 1;
		break;
	case LEFT:
		xDest -= 1;
		break;
	case RIGHT:
		xDest += 1;
		break;
	default:
		return false;
	}

	//Test collisions
	if (isWall(xDest, yDest, e->moveType()))
		return false;

	std::vector<Entity *> oldEnts;
	std::vector<Entity *> newEnts;

	getNearbyEntities(e, oldEnts);
	MapPoint mp(xDest, yDest, id);
	getNearbyEntities(&mp, newEnts);

	std::vector<Entity *> gone;
	std::vector<Entity *> appeared;
	std::vector<Entity *> remaining;

	std::sort(oldEnts.begin(), oldEnts.end());
	std::sort(newEnts.begin(), newEnts.end());

	partitionMoveResults(oldEnts, newEnts, gone, remaining, appeared);

	//Update the player as to the mobs he can see
	//TODO this should use getViewables - but screen out entities
	//		then partition results
	std::vector<Viewable *> shownStuff, hiddenStuff, retainedStuff, oldStuff, newStuff;
	getNearbyViewables(e, oldStuff, false);
	getNearbyViewables(&mp, newStuff, false);

	std::sort(oldStuff.begin(), oldStuff.end());
	std::sort(newStuff.begin(), newStuff.end());

	partitionMoveResults(oldStuff, newStuff, hiddenStuff, retainedStuff, shownStuff);

	for (Entity *other : remaining) {
		other->entityMoved(e, dir);
	}

	e->setX(xDest);
	e->setY(yDest);

	//TODO replace the character cast
	if (e->getType() == Entity::E_CHARACTER && !getPortal(e->getX(), e->getY())) {
		std::vector<Door *> nearDoors;
		getThresholdDoors(e, nearDoors);
		e->showDoors(nearDoors);
	}

	//TODO add batch show to entities
	for (Entity *o : appeared) {
		o->showViewable(e);
		e->showViewable(o);
	}
	for (Entity *o : gone) {
		o->unshowViewable(e);
		e->unshowViewable(o);
	}
	for (Viewable *v : shownStuff)
		e->showViewable(v);
	for (Viewable *v : hiddenStuff)
		e->unshowViewable(v);

	//Activate triggers
	for (auto it = triggers.begin(); it != triggers.end(); it++) {
		if ((*it)->getX() == xDest && (*it)->getY() == yDest) {
			if ((**it)(e)) {
				triggers.erase(it);
				return true;
			}
		}
	}

	return true;
}

/**
 * \brief Get the first entity on the point e
 *
 * Get the first entity on the point e, or return null if no entity is at e.
 * Here, first means that players are selected before mobs, but no other
 * guarantee of the ordering is given.
 * \param[in] e The position on the map to search for an entity at
 * \return An entity located at e, or null
 */
Entity *Map::getEnt(MapPoint *e)
{
	Entity *first = 0;

	for (auto it = entities.begin(); it != entities.end(); it++) {
		if ((*it)->getX() == e->getX() && (*it)->getY() == e->getY()) {
			if ((*it)->getType() == Entity::E_CHARACTER)
				return (*it); //always return first character if possible
			else if (first == 0)
				first = *it; //return this one if no characters found
		}
	}

	return first;
}

/**
 * \brief Returns all of the entities in a given area
 *
 * Returns all of the entities in a given area. This means all players and mobs.
 * \param area The area to search, given as a list of map points.
 * \param targets The vector to populate with targets found in the area.
 */
void Map::getEnts(const std::vector<MapPoint> &area, std::vector<Entity *> &targets, Entity *exclusion)
{
	std::for_each(entities.begin(), entities.end(), [&](Entity *m) {
		for (auto it = area.begin(); it != area.end(); it++) {
			if (it->getX() == m->getX() && it->getY() == m->getY() && m != exclusion) {
				targets.push_back(m);
			}
		}
	});
}

void Map::getNearbyEntities(MapPoint *e, std::vector<Entity *> &nearby)
{
	for (Entity *m : entities) {
		if (m->dist(e) <= NEARBY)
			nearby.push_back(m);
	}
}

void Map::addSpawner(Spawner *s)
{
	spawners.push_back(s);
}

void Map::tick(std::vector<std::function<void ()> > &deferred)
{
	//TODO run ai scripts here probably

	//TODO improve
	for (auto it = entities.begin(); it != entities.end();) {
		if ((*it)->tick(deferred)) {
			auto e = *it;
			it = entities.erase(it);
			forEachNearby(e, [&](Entity *o) {
				o->unshowViewable(e);
			});
		}
		else
			++it;
	}

	for (unsigned int i = 0; i < spawners.size(); i++)
		spawners[i]->tick();

	for (auto it = triggers.begin(); it != triggers.end();) {
		if ((*it)->tick()) {
			delete *it;
			it = triggers.erase(it);
		}
		else
			it++;
	}

	int t = time(NULL);
	while (!timers.empty() && timers.top()->getTime() <= t) {
		Timer *t = timers.top();
		timers.pop();
		if (t->trigger())
			timers.push(t);
		else
			delete t;
	}
}

void Map::getNearbyDoors(MapPoint *e, std::vector<Door *> &nearby)
{
	for (unsigned int i = 0; i < doors.size(); i++) {
		if (MH_DIST(e->getX(), doors[i].getX(), e->getY(), doors[i].getY()) <= NEARBY) {
			nearby.push_back(&(doors[i]));
		}
	}
}
void Map::getThresholdDoors(MapPoint *e, std::vector<Door *> &nearby)
{
	for (unsigned int i = 0; i < doors.size(); i++) {
		if (MH_DIST(e->getX(), doors[i].getX(), e->getY(), doors[i].getY()) == NEARBY) {
			nearby.push_back(&(doors[i]));
		}
	}
}

Door *Map::getDoor(unsigned short x, unsigned short y)
{
	for (unsigned int i = 0; i < doors.size(); i++) {
		if (doors[i].getX() == x && doors[i].getY() == y) {
			return &(doors[i]);
		}
	}
	return 0;
}

void Map::forEachNearby(MapPoint *e, std::function<void (Entity *)> fn)
{
	assert(fn);
	std::vector<Entity *> near;
	getNearbyEntities(e, near);

	std::for_each(near.begin(), near.end(), fn);
}

GroundItem *Map::getItem(unsigned short x, unsigned short y, unsigned int looterId)
{
	auto it = items.end();
	while (it != items.begin()) {
		--it;
		if ((*it)->getX() == x && (*it)->getY() == y && (*it)->canLoot(looterId)) {
			GroundItem *ret =  (*it);
			forEachNearby((*it), [&](Entity *e) {
				e->unshowViewable(*it);
			});

			items.erase(it);
			return ret;
		}
	}
	return 0;
}

bool Map::mapJump(Entity *e, unsigned short x, unsigned short y)
{
	//Is the target square free?
	if (isWall(x, y, e->moveType()) || getPortal(x, y))
		return false;

	//unshow entity wrt other characters
	MapPoint newPoint(x, y, id);

	std::vector<Entity *> oldEnts, newEnts, hiddenEnts, remainingEnts, shownEnts;
	std::vector<Viewable *> oldStuff, newStuff, hiddenStuff, remainingStuff, shownStuff;

	getNearbyEntities(e, oldEnts);
	getNearbyEntities(&newPoint, newEnts);
	getNearbyViewables(e, oldStuff, false);
	getNearbyViewables(&newPoint, newStuff, false);
	std::sort(oldEnts.begin(), oldEnts.end());
	std::sort(newEnts.begin(), newEnts.end());
	std::sort(oldStuff.begin(), oldStuff.end());
	std::sort(newStuff.begin(), newStuff.end());

	partitionMoveResults(oldEnts, newEnts, hiddenEnts, remainingEnts, shownEnts);
	partitionMoveResults(oldStuff, newStuff, hiddenStuff, remainingStuff, shownStuff);

	//move the entity
	e->setX(x);
	e->setY(y);

	//Show, then hide
	//TODO batch show
	for (Entity *o : remainingEnts)
		o->showViewable(e);
	for (Entity *o : shownEnts) {
		o->showViewable(e);
		if (o != e)
			e->showViewable(o);
	}
	for (Viewable *v : shownStuff)
		e->showViewable(v);
	//and the hide part
	for (Entity *o : hiddenEnts) {
		if (o == e)
			o->showViewable(e);
		else {
			o->unshowViewable(e);
			e->unshowViewable(o);
		}
	}
	for (Viewable *v : hiddenStuff)
		e->unshowViewable(v);

	return true;
}

bool Map::putGold(unsigned int amt, unsigned short x, unsigned short y, std::vector<unsigned int> *protection, unsigned int protectionTime)
{
	if (isWall(x, y, MOVE_NO_PORTALS | MOVE_NO_DOORS)) {
		return false;
	}
	if (!amt)
		return true;

	GroundItem *gold = new GroundItem(amt, id, x, y, protectionTime, protection);
	items.push_back(gold);
	forEachNearby(gold, [&](Entity *e) {
		e->showViewable(gold);
	});

    return true;
}
