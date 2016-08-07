/*
 * CharacterSession.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef CHARACTERSESSION_H_
#define CHARACTERSESSION_H_
#include "BasicSession.h"
#include "EncryptionService.h"
#include "Character.h"
#include "LockedChar.h"
#include <pthread.h>
#include <atomic>

class CharacterSession: public BasicSession
{
public:
    CharacterSession(int sockfd, in_addr_t ip, int now);
    virtual ~CharacterSession();

    void dataReady();
    void disconnect();

    void lock();
    void unlock();
    void sendPacket(DAPacket *p);
    Character *getCharacter()
    {
        return character;
    }
    void systemMessage(const char *msg, unsigned char channel);
    char bgm()
    {
        return curBgm;
    }
    void bgm(char bgm)
    {
        curBgm = bgm;
    }
    bool isBgmSet()
    {
        return bgmPlayerSet;
    }
    void aboutToTimeout();

private:
    EncryptionService *service;
    Character *character;
    std::atomic_uchar ordinal;
    unsigned char moveOrdinal;
    char curBgm;bool bgmPlayerSet;

    //Packet functions
    void attachCharacter(DAPacket *p);
    void getCharacterInfo();
    void getExtraInfo();
    void move(DAPacket *p);
    void talk(DAPacket *p);
    void logout(DAPacket *p);
    void unk0(DAPacket *p);
    void turn(DAPacket *p);
    void getLegendData();
    void useSkill(DAPacket *p);
    void attack();
    void movePane(DAPacket *p);
    void clicked(DAPacket *p);
    void refresh();
    void incStat(DAPacket *p);
    void useItem(DAPacket *p);
    void unequipItem(DAPacket *p);
    void dropItem(DAPacket *p);
    void pickupItem(DAPacket *p);
    void dropGold(DAPacket *p);
    void whisper(DAPacket *p);
    void resumeConv(DAPacket *p);
    void resumeConvLong(DAPacket *p);
    void countryList();
    void showBoard(DAPacket *p);
    void fieldWarp(DAPacket *p);
    void chant(DAPacket *p);
    void startCasting(DAPacket *p);
    void useSecret(DAPacket *p);
    void toggleGroup();
    void groupInvite(DAPacket *p);
    void emote(DAPacket *p);
    void giveItem(DAPacket *p);
    void trade(DAPacket *p);
    void getMap(DAPacket *p);
    void giveGold(DAPacket *p);
    void settings(DAPacket *p);

    pthread_mutex_t writelock; //must be held before writing on the socket. must not be held while locking a map(that is, a map lock must be acquired FIRST)
    //efficiency considerations: it's hard to say at the moment if an atomic would be faster (but probably would be)
};

#endif /* CHARACTERSESSION_H_ */
