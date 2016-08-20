/*
 * script.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */

#include "script.h"

log4cplus::Logger script::log()
{
	return log4cplus::Logger::getInstance("renaissance.script");
}
