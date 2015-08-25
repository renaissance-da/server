/*
 * random.h
 *
 *  Created on: 2012-12-07
 *      Author: per
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdlib.h>

#ifdef WIN32
struct drand48_data {
    unsigned short x[3];
};
int srand48_r(long int seedval, drand48_data *buffer);
int drand48_r(drand48_data *buffer, double *result);
int lrand48_r(drand48_data *buffer, long *result);
int mrand48_r(drand48_data *buffer, long *result);

#endif

long random();
double drandom();
void rngInit(drand48_data *rngdata);
long random_s();

#endif /* RANDOM_H_ */
