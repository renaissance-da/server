/*
 * lower.h
 *
 *  Created on: 2013-05-14
 *      Author: per
 */

#ifndef LOWER_H_
#define LOWER_H_
#include <algorithm>
#include <string>
#include <ctype.h>

void inline lower(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(), tolower);
}

#endif /* LOWER_H_ */
