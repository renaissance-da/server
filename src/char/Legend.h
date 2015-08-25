/*
 * Legend.h
 *
 *  Created on: 2013-06-23
 *      Author: per
 */

#ifndef LEGEND_H_
#define LEGEND_H_

#include <map>

class Legend
{
public:
    enum ParamFormat
    {
        PF_NO_PARAMS = 0,
        PF_STRING = 1,
        PF_INT = 2,
        PF_STR_INT = 3,
        PF_INT_STR = 4
    };

    Legend(int id, const char *prefix, const char *text, ParamFormat pf,
        short icon, short color);
    virtual ~Legend();

    static Legend *getById(int id);
    int getId()
    {
        return id;
    }
    const char *getPrefix()
    {
        return prefix;
    }
    int getText(char *buffer, int buflen, const char *textParam, int intParam,
        int timestamp);
    short getIcon()
    {
        return icon;
    }
    short getColor()
    {
        return color;
    }

private:
    int id;
    char prefix[5];
    char text[128];
    ParamFormat format;
    short icon, color;

    static std::map<int, Legend *> legends;

};

#endif /* LEGEND_H_ */
