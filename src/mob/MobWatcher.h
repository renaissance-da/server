/*
 * MobWatcher.h
 *
 *  Created on: May 5, 2014
 *      Author: per
 */

#ifndef MOBWATCHER_H_
#define MOBWATCHER_H_

class Mob;

class MobWatcher {

public:
	virtual void mobDied(Mob *m) = 0;
	virtual ~MobWatcher() {}
};

#endif /* MOBWATCHER_H_ */
