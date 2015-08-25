/*
 * Tracker.h
 *
 *  Created on: 2013-06-05
 *      Author: per
 */

#ifndef TRACKER_H_
#define TRACKER_H_

#include <vector>

class Tracker
{
public:
    Tracker();
    Tracker(int qid, int *mobIds, int n, int *qty);
    ~Tracker();

    int countTotal();
    int countId(int id);
    int getQid()
    {
        return qid;
    }
    void killed(int mobId, int nKills);

    class TrackerIterator
    {
    private:
        Tracker *t;
        int index;
    public:
        TrackerIterator(Tracker *t, int index)
        {
            this->t = t;
            this->index = index;
        }

        TrackerIterator next()
        {
            return TrackerIterator(t, index + 1);
        }

        bool operator==(const TrackerIterator &rhs)
        {
            return t == rhs.t && index == rhs.index;
        }

        bool operator!=(const TrackerIterator &rhs)
        {
            return !(*this == rhs);
        }

        TrackerIterator operator++()
        {
            index++;
            return *this;
        }

        std::pair<int, int> operator *()
        {
            return {t->mobIds[index], t->killCounts[index]};
        }
    };

    TrackerIterator begin()
    {
        return TrackerIterator(this, 0);
    }

    TrackerIterator end()
    {
        int index = 0;
        if (!qid) {
            while (index < nMobs && mobIds[index])
                index++;
        }
        else {
            index = nMobs;
        }

        return TrackerIterator(this, index);
    }

private:
    int qid, ring;
    int nMobs;
    int *mobIds;
    int *killCounts;
};

#endif /* TRACKER_H_ */
