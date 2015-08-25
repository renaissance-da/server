/*
 * SecretInfo.h
 *
 *  Created on: 2013-03-05
 *      Author: per
 */

#ifndef SECRETINFO_H_
#define SECRETINFO_H_

#include "SkillInfo.h"

class SecretInfo: public SkillInfo
{
public:
    //SecretInfo();
    //~SecretInfo();

    unsigned char nlines;
    bool targets;
    char targetDesc[128];
};

#endif /* SECRETINFO_H_ */
