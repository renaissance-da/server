/*
 * Hash.h
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#ifndef HASH_H_
#define HASH_H_

#include <stdint.h>

//PRE: result_256 is a 256 bit array
void hashPw(char const *pw, uint32_t *result_256, const char *salt);
void md5(char const *name, uint32_t *result_128, unsigned int len);
char *hashName(char const *name);
void initSalt(char *salt);

#endif /* HASH_H_ */
