/*
 * LoginSession.h
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#ifndef LOGINSESSION_H_
#define LOGINSESSION_H_

#include "EncryptionService.h"
#include "BasicSession.h"
#include <stdlib.h>
#include <string>
#include "log4cplus/logger.h"

class LoginSession: public BasicSession
{
public:

    LoginSession(int fd, in_addr_t ip, int now);
    virtual ~LoginSession();

    void dataReady();

    static void initMessages();

private:
    EncryptionService *service;
    unsigned char ordinal;
    std::string editName;

    void startEncrypt();
    void confirmEncrypt(DAPacket *p);
    void queryHomepage();
    void createCharacter(DAPacket *p);
    void setAttributes(DAPacket *p);
    void changePassword(DAPacket *p);
    void login(DAPacket *p);
    bool checkVersion(DAPacket *p);
    void loggedout(DAPacket *p);
    void aboutToTimeout()
    {
        lastRecv = now;
    } //dont time out
    void getMotd();

    static log4cplus::Logger log;
};

#endif /* LOGINSESSION_H_ */
