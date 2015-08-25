/*
 * StandardAI.h
 *
 *  Created on: 2013-06-13
 *      Author: per
 */

#ifndef STANDARDAI_H_
#define STANDARDAI_H_

#include "MobAI.h"
#include "defines.h"

class StandardAI: public MobAI {
public:
	StandardAI(Mob *mob, int subMode);
	virtual ~StandardAI();

	void runAI();
	void confuse();
	void taunt(Entity *e);

private:
	int rethinkDelay, actDelay, clearDelay, maxConfuses;
	bool angry, sleepy, hostile;
};

#endif /* STANDARDAI_H_ */
