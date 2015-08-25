/*
 * ScriptTimer.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: per
 */

#include <ScriptTimer.h>
#include "npc_bindings.h"

ScriptTimer::ScriptTimer(int scriptId, int time):
Timer(time),
scriptId(scriptId)
{
}

bool ScriptTimer::trigger()
{
	int next = scriptTimer(scriptId);
	if (next > 0) {
		changeTime(next);
		return true;
	}
	return false;
}
