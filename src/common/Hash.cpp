/*
 * hash.cpp
 *
 *  Created on: 2011-09-10
 *      Author: per
 */

#include "Hash.h"
#include <string.h>
#include "random_engines.h"

const unsigned int k[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa,
    0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138,
    0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624,
    0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f,
    0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

//Right rotate operation
inline uint32_t rrotate_nocarry(uint32_t x, unsigned int amt)
{
    return (x >> amt) | (x << (32 - amt));
}

//PRE: result_256 is a 256 bit array
//		salt is a 32 byte array
void hashPw(char const *pw, uint32_t *result_256, char const *salt)
{

    //Simplified version of sha-256 (simplifications based on input 4 to 8 bytes)
    unsigned int pwLen = strlen(pw);
    unsigned int i;
    char buf[40];
    memcpy(buf, salt, 32);
    memcpy(buf + 32, pw, pwLen);
    pwLen += 32;
    pw = buf;

    uint32_t *h = result_256;
    h[0] = 0x6a09e667;
    h[1] = 0xbb67ae85;
    h[2] = 0x3c6ef372;
    h[3] = 0xa54ff53a;
    h[4] = 0x510e527f;
    h[5] = 0x9b05688c;
    h[6] = 0x1f83d9ab;
    h[7] = 0x5be0cd19;

    uint32_t *words = new uint32_t[64];
    for (i = 0; i < 15; i++) {
        words[i] = 0;
    }
    words[15] = pwLen * 8;
    for (i = 0; i < pwLen; i++) {
        words[i / 4] |= (unsigned char) (pw[i]) << (8 * (3 - (i % 4)));
    }
    words[i / 4] |= 0x80 << (8 * (3 - (i % 4)));
    for (i = 16; i < 64; i++) {
        uint32_t s0 = rrotate_nocarry(words[i - 15], 7)
            ^ rrotate_nocarry(words[i - 15], 18) ^ (words[i - 15] >> 3);
        uint32_t s1 = rrotate_nocarry(words[i - 2], 17)
            ^ rrotate_nocarry(words[i - 2], 19) ^ (words[i - 2] >> 10);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }

    uint32_t res[8];
    for (i = 0; i < 8; i++) {
        res[i] = h[i];
    }
    for (int i = 0; i < 64; i++) {
        uint32_t s0 = rrotate_nocarry(res[0], 2) ^ rrotate_nocarry(res[0], 13)
            ^ rrotate_nocarry(res[0], 22);
        uint32_t maj = (res[0] & res[1]) ^ (res[0] & res[2])
            ^ (res[1] & res[2]);
        uint32_t t2 = s0 + maj;
        uint32_t s1 = rrotate_nocarry(res[4], 6) ^ rrotate_nocarry(res[4], 11)
            ^ rrotate_nocarry(res[4], 25);
        uint32_t ch = (res[4] & res[5]) ^ ((~res[4]) & res[6]);
        uint32_t t1 = res[7] + s1 + ch + k[i] + words[i];

        res[7] = res[6];
        res[6] = res[5];
        res[5] = res[4];
        res[4] = res[3] + t1;
        res[3] = res[2];
        res[2] = res[1];
        res[1] = res[0];
        res[0] = t1 + t2;
    }
    for (int i = 0; i < 8; i++) {
        h[i] += res[i];
    }
    delete[] words;
}

//PRE: salt is a 32 byte array
void initSalt(char *salt)
{
    uint32_t *usalt = (uint32_t *) salt;
    for (int i = 0; i < 8; i++) {
	usalt[i] = (uint32_t)generator()();
    }
}
