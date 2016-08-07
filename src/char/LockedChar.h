/*
 * LockedChar.h
 *
 *  Created on: Jul 13, 2016
 *      Author: per
 */

#ifndef SRC_CHAR_LOCKEDCHAR_H_
#define SRC_CHAR_LOCKEDCHAR_H_

#include "Character.h"

class LockedChar {
public:
	LockedChar(Character *c);
	~LockedChar();

	Character *operator->() {
		return c;
	}

	Character *get() {
		return c;
	}

private:
	Character *c;
	Map *m;
};

#endif /* SRC_CHAR_LOCKEDCHAR_H_ */
