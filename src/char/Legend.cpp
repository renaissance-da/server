/*
 * Legend.cpp
 *
 *  Created on: 2013-06-23
 *      Author: per
 */

#include "Legend.h"
#include <stdio.h>
#include <string.h>
#include "GameTime.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

std::map<int, Legend *> Legend::legends;

Legend::Legend(int id, const char *prefix, const char *text, ParamFormat pf,
    short icon, short color) :
    id(id), format(pf), icon(icon), color(color)
{
    strcpy(this->prefix, prefix);
    strcpy(this->text, text);

    legends[id] = this;
}

Legend::~Legend()
{
    auto it = legends.find(id);

    if (it != legends.end() && it->second == this) {
        legends.erase(it);
    }
}

Legend *Legend::getById(int id)
{
    auto it = legends.find(id);

    if (it != legends.end())
        return it->second;
    else
        return 0;
}

int Legend::getText(char *buffer, int buflen, const char *textParam,
    int intParam, int timestamp)
{
    int usedLen;
    switch (format) {
    case PF_STRING:
        usedLen = snprintf(buffer, buflen, text, textParam);
        break;
    case PF_INT:
        usedLen = snprintf(buffer, buflen, text, intParam);
        break;
    case PF_STR_INT:
        usedLen = snprintf(buffer, buflen, text, textParam, intParam);
        break;
    case PF_INT_STR:
        usedLen = snprintf(buffer, buflen, text, intParam, textParam);
        break;
    case PF_NO_PARAMS:
    default:
        usedLen = snprintf(buffer, buflen, text, 0);
        break;
    }

    if (usedLen >= buflen - 3)
        return buflen; //Too long

    buffer[usedLen] = ' ';
    buffer[usedLen + 1] = '-';
    buffer[usedLen + 2] = ' ';

    usedLen += 3
        + gameDateseason(buffer + usedLen + 3, buflen - usedLen - 3, timestamp);
    if (usedLen > buflen)
        return buflen;
    return usedLen;
}
