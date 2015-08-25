/*
 * LockSet.cpp
 *
 *  Created on: Aug 8, 2015
 *      Author: per
 */

#include "LockSet.h"

__thread std::set<int> *locks;

/**
 * Add the given lock ID to this thread's set of held locks.
 * \param[in] lockId A unique identifier representing the lock.
 * \return True if lockId is smaller than every currently held lock's ID,
 * false otherwise.
 */
bool LockSet::addLock(int lockId)
{
    if (!locks) {
        locks = new std::set<int>;
    }
    auto it = locks->rbegin();
    bool r = it == locks->rend() || *it < lockId;
    locks->insert(lockId);
    return r;
}

/**
 * Remove a lock ID from this thread's set of held locks.
 * \param[in] lockId A unique identifier representing the lock.
 * \return True if this thread previously held the given ID, false otherwise.
 */
bool LockSet::removeLock(int lockId)
{
    if (!locks) {
        locks = new std::set<int>;
    }
    auto it = locks->find(lockId);
    if (it != locks->end()) {
        locks->erase(it);
        return true;
    }
    return false;
}
