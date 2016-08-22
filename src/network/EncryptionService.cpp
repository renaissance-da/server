/*
 * EncryptionService.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */
#include "defines.h"

#include "EncryptionService.h"
#include "Hash.h"
#include <stdio.h>
#include <string.h>
#include "random_engines.h"
#include "defines.h"
#include <assert.h>
#include <fstream>

char **loadTables()
{
    std::ifstream seeds("seedTables", std::ios::binary);
    if (!seeds.is_open())
        return 0;
    char **seedTables = new char *[10];
    for (int i = 0; i < 10; i++) {
        seedTables[i] = new char[0x100];
        seeds.read(seedTables[i], 0x100);
    }

    return seedTables;
}

char **EncryptionService::seedTables = loadTables();

EncryptionService::EncryptionService(char const *name, char const *key,
    unsigned char keyLen, unsigned char packetTable)
{
    assert(keyLen == 9);
    this->keyLen = keyLen;
    seedTable = packetTable;
    memcpy(this->key, key, keyLen);

    //736
    keyTable = hashName(name);
}

// Constructs a randomized encryption service
EncryptionService::EncryptionService() :
    keyLen(9), keyTable(0)
//key({ 'U', 'r', 'k', 'c', 'n', 'I', 't', 'n', 'I' })
{
    //TODO fix table 9
    std::uniform_int_distribution<int> seed_dist(0, 8);
    std::uniform_int_distribution<int> byte_dist(0, 255);
    seedTable = seed_dist(generator());

    for (int i = 0; i < keyLen; i++) {
        key[i] = (char)byte_dist(generator());
    }
}

EncryptionService::~EncryptionService()
{
    if (keyTable)
        delete[] keyTable;
}

void EncryptionService::cypher2(DAPacket *p)
{
    //old style decryption for use with login server
    p->cyphered();

    unsigned short len = p->getDataLen();
    char *data = p->getDataPtr();
    unsigned char ordinal = p->getOrd();
    for (unsigned short i = 0; i < len; i++) {
        if ((i / keyLen) % 0x100 != ordinal)
            data[i] = data[i] ^ key[i % keyLen]
                ^ seedTables[seedTable][(i / keyLen) % 0x100]
                ^ seedTables[seedTable][ordinal];
        else
            data[i] = data[i] ^ key[i % keyLen]
                ^ seedTables[seedTable][ordinal];
    }

}

void EncryptionService::decrypt(DAPacket *p)
{
    switch (p->getCode()) {
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x0A: //Unconfirmed
    case 0x0B: //Unconfirmed
    case 0x26:
    case 0x2D: //Unconfirmed
    case 0x3A: //Unconfirmed
    case 0x42: //Unconfirmed
    case 0x43: //Unconfirmed
    case 0x4B: //Unconfirmed
    case 0x57:
    case 0x62: //Unconfirmed
    case 0x68:
    case 0x71: //Unconfirmed
    case 0x73: //Unconfirmed
    case 0x7B: //Unconfirmed/irrelevant
        cypher2(p);
        break;
    default:
        if (keyTable)
            cypher(p, 0x7470, 0x23);
        else
            throw E_NOKEYTABLE;
        break;
    }
}

void EncryptionService::encrypt(DAPacket *p)
{
	std::uniform_int_distribution<int> dist(0, 255);

    p->appendByte((char)dist(generator()));
    p->appendByte((char)dist(generator()));
    p->appendByte((char)dist(generator()));

    switch (p->getCode()) {
    case 0x01: //unverified
    case 0x02:
    case 0x0A:
    case 0x56: //unverified
    case 0x60:
    case 0x62: //unverified
    case 0x66:
    case 0x6F:
        cypher2(p);
        break;
    default:
        if (keyTable)
            cypher(p, 0x6474, 0x24);
        else
            throw E_NOKEYTABLE;
        break;
    }
}

void EncryptionService::cypher(DAPacket *p, unsigned short k0_mask,
    unsigned char k1_mask)
{
    p->cyphered();

    char key[9];
    unsigned char *keyRoot = (unsigned char *) p->getKeyRoot();

    const int k0 = (keyRoot[0] + (keyRoot[2] << 8)) ^ k0_mask;
    const int k1 = (keyRoot[1] ^ k1_mask);

#ifndef NDEBUG
    char dummykey[9];
    for (int i = 0; i < 9; i++) {
        int a = (k1 * k1 + i * 9) * i;
        int b = k0 + a;
        b &= 0x800003FF;
        if (b < 0)
            b = ((b - 1) | 0xFFFFFC00) + 1;
        dummykey[i] = keyTable[b];
    }
#endif

    //Should give same results
    //Note that b cannot become signed because b = k0 + (k1*k1 + 9i)i <= 0xFFFF + (0xFF^2 + 0x51)9 = 9F0E1
    //given that key lengths must be nine. for other lengths, b could become signed
    int a = k0;
    int b = k1 * k1 - 9;
    key[0] = keyTable[k0 & 0x3FF];
    for (int i = 1; i < 9; i++) {
        b += 18;
        a += b;
        key[i] = keyTable[a & 0x3FF];
    }
    assert(!strncmp(key, dummykey, 9));

    unsigned short len = p->getDataLen();
    char *data = p->getDataPtr();
    unsigned char ordinal = p->getOrd();

    for (unsigned short i = 0; i < len; i++) {
        if ((i / 9) % 0x100 != ordinal)
            data[i] = data[i] ^ key[i % 9]
                ^ seedTables[seedTable][(i / keyLen) % 0x100]
                ^ seedTables[seedTable][ordinal];
        else
            data[i] = data[i] ^ key[i % 9] ^ seedTables[seedTable][ordinal];
    }
}

std::unique_ptr<DAPacket> EncryptionService::getSetPacket(int slistCrc)
{
    char data[] = { 0 };
    
    std::unique_ptr<DAPacket> p(new DAPacket(Server::ENCRYPT_DATA, data, 1));
    p->appendInt(slistCrc);
    p->appendByte(seedTable);
    p->appendString(keyLen, key);

    /*char *data = new char[7 + keyLen];
    data[0] = 0;
    data[1] = 0xf0;
    data[2] = 0xd7;
    data[3] = 0x69;
    data[4] = 0x14;
    data[5] = seedTable;
    data[6] = keyLen;
    for (unsigned short i = 0; i < keyLen; i++) {
        data[7 + i] = key[i];
    }
    DAPacket *p = new DAPacket(Server::ENCRYPT_DATA, data, 7 + keyLen);
    delete[] data;*/
    return p;
}

unsigned char EncryptionService::getSeed()
{
    return seedTable;
}

unsigned char EncryptionService::getKeyLen()
{
    return keyLen;
}

char const *EncryptionService::getKey()
{
    return key;
}
