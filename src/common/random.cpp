/*
 * random.cpp
 *
 *  This includes re-implementations of the drand48 type algorithms
 *
 *  Created on: 2012-12-07
 *      Author: per
 */

#include "random.h"

//TODO this should be TLS for windows
#ifndef WIN32
__thread
#endif
drand48_data *rng;

/**
 * \brief Returns a random number between 0 and 2^31
 *
 * Returns a random number between 0 and 2^31. Uses lrand48_r
 * \return A random number between 0 and 2^31
 */
long random()
{
    long result;
    lrand48_r(rng, &result);
    return result;
}

double drandom()
{
    double result;
    drand48_r(rng, &result);
    return result;
}

void rngInit(drand48_data *rngdata)
{
    rng = rngdata;
}

long random_s()
{
    long result;
    mrand48_r(rng, &result);
    return result;
}

#ifdef WIN32

unsigned long long lcong(drand48_data *buffer)
{
    unsigned long long res = 0xb;
    const unsigned long long a = 0x5deece66d;
    res += buffer->x[0] * a;
    res += (buffer->x[1] * a) << 16;
    res += (buffer->x[2] * a) << 32;

    buffer->x[0] = res & 0xffff;
    buffer->x[1] = (res >> 16) & 0xffff;
    buffer->x[2] = (res >> 32) & 0xffff;

    return res & 0xffffffffffff;
}

int srand48_r(long int seedval, drand48_data *buffer)
{
    buffer->x[0] = 0x330e;
    buffer->x[1] = seedval & 0xffff;
    buffer->x[2] = (seedval >> 16) & 0xffff;
    return 0;
}
int drand48_r(drand48_data *buffer, double *result)
{
    *result = lcong(buffer) / 281474976710656.0;
    return 0;
}
int lrand48_r(drand48_data *buffer, long *result)
{
    unsigned long long res = lcong(buffer);
    *result = res >> 17;
    return 0;
}
int mrand48_r(drand48_data *buffer, long *result)
{
    unsigned long long res = lcong(buffer);
    int sr = (int)(res >> 16);
    *result = sr;

    return 0;
}
#endif
