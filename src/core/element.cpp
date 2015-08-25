/*
 * element.cpp
 *
 *  Created on: 2013-04-25
 *      Author: per
 */
#include "element.h"

const char *getEleName(Element e)
{
    switch (e) {
    case None:
        return "None";
    case Fire:
        return "Fire";
    case Water:
        return "Water";
    case Wind:
        return "Wind";
    case Earth:
        return "Earth";
    case Light:
        return "Light";
    case Dark:
        return "Dark";
    case Wood:
        return "Wood";
    case Metal:
        return "Metal";
    default:
        return "Nature";
    }
}
