/*
 * MapPoint.h
 *
 *  Created on: 2012-12-23
 *      Author: per
 */

#ifndef MAPPOINT_H_
#define MAPPOINT_H_

#define IS_WALL(X,Y) (walls[(X) + (Y) * width])
#define DIFF(x,y) ((x) >= (y) ? (x) - (y) : (y) - (x))
#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#define MH_DIST(x1, x2, y1, y2) (DIFF((x1),(x2)) + DIFF((y1),(y2)))

class MapPoint
{
public:
    MapPoint(unsigned short x, unsigned short y, unsigned short map);
    ~MapPoint();

    unsigned short getX() const
    {
        return x;
    }
    unsigned short getY() const
    {
        return y;
    }
    unsigned short getMapId()
    {
        return map;
    }
    void setX(unsigned short x)
    {
        this->x = x;
    }
    void setY(unsigned short y)
    {
        this->y = y;
    }
    void setMap(unsigned short m)
    {
        map = m;
    }
    unsigned int dist(const MapPoint *other);

private:
    unsigned short x, y, map;
};

#endif /* MAPPOINT_H_ */
