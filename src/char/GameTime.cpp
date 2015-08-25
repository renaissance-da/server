/*
 * GameTime.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#include "GameTime.h"
#include <time.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

void getGameTime(int &deoch, int &season, int &moon, int &sun, int &hour, int t)
{
    deoch = t / 3942000;
    t %= 3942000;
    season = t / 985500;
    moon = (t / 303231) + 1;
    t %= 303231;
    sun = (t / 10830) + 1;
    t %= 10830;
    hour = t / 452;
}

void getGameTime(int &deoch, int &season, int &moon, int &sun, int &hour)
{
    getGameTime(deoch, season, moon, sun, hour, time(NULL));
}

const char *getSeason(int season)
{
    switch (season) {
    case 0:
        return "Winter";
    case 1:
        return "Spring";
    case 2:
        return "Summer";
    default:
        return "Fall";
    }
}

const char *getSuffix(int n)
{
    switch (n % 10) {
    case 1:
        return "st";
    case 2:
        return "nd";
    case 3:
        return "rd";
    default:
        return "th";
    }
}

int gameDatetime(char *buf, int len)
{
    int deoch, season, moon, sun, hour;
    getGameTime(deoch, season, moon, sun, hour);

    if (hour == 0) {
        return snprintf(buf, len,
            "Deoch %d, %d%s Moon, %d%s Sun, Midnight (POSIX reckoning)", deoch,
            moon, getSuffix(moon), sun, getSuffix(sun));
    }
    else if (hour == 12) {
        return snprintf(buf, len,
            "Deoch %d, %d%s Moon, %d%s Sun, Noon (POSIX reckoning)", deoch,
            moon, getSuffix(moon), sun, getSuffix(sun));
    }
    else if (hour < 12) {
        return snprintf(buf, len,
            "Deoch %d, %d%s Moon, %d%s Sun, %d a.m. (POSIX reckoning)", deoch,
            moon, getSuffix(moon), sun, getSuffix(sun), hour);
    }
    else {
        return snprintf(buf, len,
            "Deoch %d, %d%s Moon, %d%s Sun, %d p.m. (POSIX reckoning)", deoch,
            moon, getSuffix(moon), sun, getSuffix(sun), hour - 12);
    }
}
int gameDateseason(char *buf, int len, int timestamp)
{
    int deoch, season, moon, sun, hour;
    getGameTime(deoch, season, moon, sun, hour, timestamp);

    return snprintf(buf, len, "Deoch %d, %s", deoch, getSeason(season));
}

int gameDateseason(char *buf, int len)
{
    return gameDateseason(buf, len, time(NULL));
}

