/*
 * BaseEffect.h
 *
 *  Created on: 2013-05-09
 *      Author: per
 */

#ifndef BASEEFFECT_H_
#define BASEEFFECT_H_

#include <map>

class BaseEffect
{
public:
    BaseEffect(int id, int kind, int duration, unsigned short icon,
        const char *received, const char *ended, int param1, int param2);
    ~BaseEffect();

    int id, kind, duration, param1, param2;
    char received[128];
    char ended[128];
    char conflict[128];
    unsigned short icon;

    static BaseEffect *getById(int id);

private:
    static std::map<int, BaseEffect *> effects;
};

#endif /* BASEEFFECT_H_ */
