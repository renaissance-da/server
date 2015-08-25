/*
 * GameTime.h
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef GAMETIME_H_
#define GAMETIME_H_

void getGameTime(int &deoch, int &season, int &moon, int &sun, int &hour);
void getGameTime(int &deoch, int &season, int &moon, int &sun, int &hour,
    int t);
const char *getSeason(int season);

int gameDatetime(char *buf, int len);
int gameDateseason(char *buf, int len);
int gameDateseason(char *buf, int len, int timestamp);

#endif /* GAMETIME_H_ */
