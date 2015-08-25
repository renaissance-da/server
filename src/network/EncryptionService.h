/*
 * EncryptionService.h
 *
 * Handles the encryption of DA packets
 * The basic idea is to XOR each byte with a byte from the key (typically 9 bytes long), one of the seed tables (256 bytes long)
 * and the ordinal (one byte long)
 * Not all packets are intended for encryption, and there is nothing within an individual packet to indicate if it has
 * been encrypted or not
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef ENCRYPTIONSERVICE_H_
#define ENCRYPTIONSERVICE_H_
#include "DAPacket.h"
#include <stdlib.h>
#include <memory>

class EncryptionService
{
public:
    EncryptionService();
    EncryptionService(char const *name, char const *key, unsigned char keyLen,
        unsigned char packetTable);
    virtual ~EncryptionService();

    void decrypt(DAPacket *p);
    void encrypt(DAPacket *p);

    std::unique_ptr<DAPacket> getSetPacket(int slistCrc);
    unsigned char getSeed();
    unsigned char getKeyLen();
    char const *getKey();

private:
    char key[9];
    unsigned char keyLen;
    unsigned char seedTable;
    char *keyTable;

    //736 standard encryption
    void cypher(DAPacket *p, unsigned short k0_mask, unsigned char k1_mask);

    //pre 736 standard encryption
    void cypher2(DAPacket *p);

    static char **seedTables;

};

#endif /* ENCRYPTIONSERVICE_H_ */
