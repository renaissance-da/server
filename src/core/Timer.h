/*
 * Timer.h
 *
 *  Created on: Jul 2, 2014
 *      Author: per
 */

#ifndef TIMER_H_
#define TIMER_H_

class Timer
{
public:
    class TimerCompare
    {
    public:
        bool operator()(Timer *a, Timer *b)
        {
            return a->time > b->time;
        }
    };

    Timer(int when);
    virtual ~Timer();

    virtual bool trigger() = 0;
    int getTime()
    {
        return time;
    }
    void changeTime(int time);

private:
    int time;
};

#endif /* TIMER_H_ */
