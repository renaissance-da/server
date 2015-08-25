/*
 * Timer.cpp
 *
 *  Created on: Jul 2, 2014
 *      Author: per
 */

#include <Timer.h>

Timer::Timer(int when) :
    time(when)
{
}

Timer::~Timer()
{
}

void Timer::changeTime(int time)
{
    this->time = time;
}
