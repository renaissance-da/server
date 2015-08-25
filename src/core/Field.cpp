/*
 * Field.cpp
 *
 *  Created on: 2013-02-26
 *      Author: per
 */

#include "Field.h"

std::map<int, Field *> Field::fields;

Field::Field(int id)
{
    fields[id] = this;
}

Field::~Field()
{
    // TODO Auto-generated destructor stub
}

