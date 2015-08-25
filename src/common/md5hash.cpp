/*
 * md5hash.cpp
 *
 *  Created on: 2012-12-04
 *      Author: per
 */

#include "Hash.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int r[64] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16, 23, 4,
    11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6,
    10, 15, 21, 6, 10, 15, 21 };

const unsigned int k[64] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
    0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453,
    0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681,
    0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5,
    0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0,
    0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

int lrot(uint32_t x, int c)
{
    return (x << c) | (x >> (32 - c));
}

//result is 128 bits = 4 ints
void md5(char const *data, uint32_t *result_128, unsigned int len)
{
    int c1 = 0x67452301;
    int c2 = 0xefcdab89;
    int c3 = 0x98badcfe;
    int c4 = 0x10325476;

    //Prepare data
    //First need to compute the number of blocks
    //The final len will be at least 3 more than the data len, and must be congruent to 0 mod 64
    //int finalLen = (len+3) + ((64 - ((len+3)%64))%64);
    int finalLen = (len + 9) + ((64 - ((len + 9) % 64)) % 64);
    assert(finalLen - 9 >= (int )len);
    assert(finalLen % 64 == 0);

    uint32_t *words = (uint32_t*) calloc(finalLen, 1);
    memcpy(words, data, len);
    ((char*) words)[len] = 0x80;
    words[finalLen / 4 - 2] = len << 3;
    words[finalLen / 4 - 1] = len >> 29;

    int f, g, t;
    int a, b, c, d;

    for (int j = 0; j < finalLen / 64; j++) {
        a = c1;
        b = c2;
        c = c3;
        d = c4;

        for (int i = 0; i < 64; i++) {
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }

            t = d;
            d = c;
            c = b;
            b = b + lrot(a + f + k[i] + words[g + j * 16], r[i]);
            a = t;
        }
        c1 += a;
        c2 += b;
        c3 += c;
        c4 += d;
    }

    result_128[0] = c1;
    result_128[1] = c2;
    result_128[2] = c3;
    result_128[3] = c4;

    free(words);
}

const char hex_tbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f' };
//buffer size is 32 bytes at least
void stringify(char* buffer, uint32_t *h)
{
    for (int i = 0; i < 4; i++) {
        buffer[8 * i] = hex_tbl[(h[i] & 0xF0) >> 4];
        buffer[8 * i + 1] = hex_tbl[h[i] & 0x0F];
        buffer[8 * i + 2] = hex_tbl[(h[i] & 0xF000) >> 12];
        buffer[8 * i + 3] = hex_tbl[(h[i] & 0x0F00) >> 8];
        buffer[8 * i + 4] = hex_tbl[(h[i] & 0xF00000) >> 20];
        buffer[8 * i + 5] = hex_tbl[(h[i] & 0x0F0000) >> 16];
        buffer[8 * i + 6] = hex_tbl[(h[i] & 0xF0000000) >> 28];
        buffer[8 * i + 7] = hex_tbl[(h[i] & 0x0F000000) >> 24];
    }
}

char *hashName(char const* name)
{
    //char *res = (char *)calloc(0x20, 0x20);
    char *res = new char[1024];
    uint32_t h[4];
    md5(name, h, strlen(name));
    char initial_buf[32];
    stringify(initial_buf, h);

    md5(initial_buf, h, 32);
    stringify(res, h);

    for (int i = 0; i < 0x1F; i++) {
        //md5(next_block, h);
        md5(res, h, (i + 1) * 32);
        stringify((res + (i + 1) * 32), h);
    }

    return res;
}
