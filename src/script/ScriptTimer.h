/*
 * ScriptTimer.h
 *
 *  Created on: Sep 3, 2014
 *      Author: per
 */

#ifndef SCRIPTTIMER_H_
#define SCRIPTTIMER_H_

#include <Timer.h>

class ScriptTimer: public Timer {
public:
	ScriptTimer(int scriptId, int time);

	bool trigger() override;

private:
	int scriptId;
};

#endif /* SCRIPTTIMER_H_ */
