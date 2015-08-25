/*
 * Tracker.cpp
 *
 *  Created on: 2013-06-05
 *      Author: per
 */

#include "Tracker.h"

Tracker::Tracker() :
    qid(0), ring(0), nMobs(10)
{
    //Generic tracker
    killCounts = new int[10];
    mobIds = new int[10];

    for (int i = 0; i < 10; i++) {
        killCounts[i] = mobIds[i] = 0;
    }
}

Tracker::Tracker(int qid, int *mobIds, int n, int *qty) :
    qid(qid), ring(0), nMobs(n), mobIds(mobIds)
{
    killCounts = new int[n];

    if (qty)
        killCounts = qty;
    else {
        for (int i = 0; i < n; i++) {
            killCounts[i] = 0;
        }
    }
}

Tracker::~Tracker()
{
    delete[] killCounts;
    delete[] mobIds;
}

int Tracker::countTotal()
{
    int ret = 0;
    for (int i = 0; i < nMobs; i++) {
        ret += killCounts[i];
    }
    return ret;
}

int Tracker::countId(int id)
{
    for (int i = 0; i < nMobs; i++) {
        if (mobIds[i] == id)
            return killCounts[i];
    }
    return 0;
}

void Tracker::killed(int mobId, int nKills)
{
    for (int i = 0; i < nMobs; i++) {
        if (mobIds[i] == mobId) {
            killCounts[i] += nKills;
            return;
        }
    }

    if (!qid) {
        mobIds[ring] = mobId;
        killCounts[ring] = nKills;
        ring = (ring + 1) % 10;
    }
}

