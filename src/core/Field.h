/*
 * Field.h
 *
 *  Created on: 2013-02-26
 *      Author: per
 */

#ifndef FIELD_H_
#define FIELD_H_

#include <vector>
#include <map>

class Field
{
public:
    Field(int id);
    ~Field();

    struct FieldDest
    {
        short mapId;
        char name[128];
        unsigned short x, y;
        unsigned short xdest, ydest;
    };

    std::vector<FieldDest> dests;
    static std::map<int, Field *> fields;
    char name[128];
};

#endif /* FIELD_H_ */
