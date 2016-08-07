/*
 * LockedChar.cpp
 *
 *  Created on: Jul 13, 2016
 *      Author: per
 */

#include "LockedChar.h"

LockedChar::LockedChar(Character *character) {
	c = character;

	// lock map
	m = c->getMap();
	m->lock();

	while (m != c->getMap()) {
		m->unlock();
		m = c->getMap();
		m->lock();
	}
}

LockedChar::~LockedChar() {
	m->unlock();
}

