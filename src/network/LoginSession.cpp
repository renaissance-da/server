/*
 * LoginSession.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "LoginSession.h"
#include "DAPacket.h"
#include "DataService.h"
#include <string>
#include <string.h>
#include "defines.h"
#include "config.h"
#include "log4cplus/loggingmacros.h"
#include <stdlib.h>
#include "crc.h"

log4cplus::Logger LoginSession::log = log4cplus::Logger::getInstance(
    "renaissance.login");

const unsigned char *srvNamelist;
int slistCrc = 0;
int slistLen;

const unsigned char motd_crc[] = { 0, 0xD9, 0x15, 0x52, 0x8E };

const unsigned char motd[] = { 0x01, 0x02, 0x08, 0x78, 0x9C, 0xD5, 0x55, 0x4D,
    0x6F, 0xDB, 0x30, 0x0C, 0x3D, 0xFB, 0x5F, 0xF0, 0xB4, 0x5C, 0xB2, 0xFC,
    0x80, 0x02, 0x3D, 0x78, 0x6D, 0x30, 0x64, 0x5D, 0xDB, 0xA0, 0x0D, 0x56,
    0xEC, 0x48, 0xCB, 0xB4, 0xA3, 0x45, 0x16, 0x03, 0x4A, 0x8A, 0x67, 0xF4,
    0xCF, 0x8F, 0x4A, 0x9A, 0xE5, 0xD0, 0x0C, 0x6B, 0xBB, 0x43, 0x33, 0xDE,
    0x28, 0x8B, 0x8F, 0x5F, 0x4F, 0xCF, 0x0F, 0xE4, 0x0C, 0x77, 0x04, 0x91,
    0xE1, 0x12, 0x65, 0x05, 0x65, 0x4B, 0x01, 0x5E, 0x6B, 0xC5, 0xE4, 0xD5,
    0x21, 0xFF, 0x89, 0x15, 0x87, 0xA1, 0xD8, 0x00, 0x2D, 0x6F, 0x48, 0x3C,
    0xD5, 0x50, 0x0D, 0x10, 0x97, 0x04, 0x29, 0x90, 0x00, 0xB6, 0x42, 0xD4,
    0x91, 0x8F, 0xC0, 0x0D, 0x5C, 0x49, 0x82, 0x99, 0x8F, 0x24, 0x68, 0xA2,
    0xDD, 0x90, 0xCE, 0x65, 0xD6, 0xC0, 0xC0, 0x69, 0x77, 0x2B, 0x4F, 0x39,
    0xC7, 0x1D, 0x42, 0xD4, 0xF3, 0x60, 0x9C, 0x35, 0x2B, 0x18, 0xDD, 0x5E,
    0x8D, 0xFE, 0x65, 0x8E, 0xA7, 0xB2, 0x84, 0xE2, 0xA9, 0xE3, 0x9A, 0xC1,
    0x73, 0xDC, 0xF5, 0x3A, 0xDE, 0xF7, 0x78, 0x81, 0xDE, 0x90, 0x1B, 0x41,
    0x45, 0x8E, 0xFB, 0xB7, 0x15, 0x7C, 0x02, 0x7D, 0x16, 0x8F, 0xE7, 0xE6,
    0x66, 0xFA, 0x00, 0xE5, 0xDD, 0xB4, 0xBC, 0x3F, 0x7B, 0x33, 0xCA, 0x75,
    0xB2, 0xC1, 0x0A, 0x7C, 0xB3, 0xCE, 0x61, 0x4B, 0xF0, 0x11, 0x16, 0x82,
    0xD6, 0x5B, 0xDF, 0xC2, 0x67, 0xE1, 0xE4, 0xEB, 0xF0, 0x92, 0x4E, 0x8B,
    0x4F, 0xE8, 0xF2, 0x50, 0x6B, 0x28, 0x85, 0x3C, 0x2A, 0x8A, 0x63, 0x83,
    0x51, 0x7D, 0x8C, 0x70, 0xCF, 0x29, 0x2E, 0x61, 0x2E, 0xB8, 0x51, 0xE7,
    0x02, 0xB7, 0x84, 0x3C, 0x8E, 0x52, 0x76, 0xE8, 0xE1, 0x4B, 0xF2, 0xAD,
    0x23, 0xF8, 0x00, 0xDF, 0xB9, 0xB7, 0x4A, 0xCC, 0x05, 0x89, 0xD8, 0xC8,
    0x32, 0xBC, 0x70, 0xE4, 0x27, 0xB0, 0x9A, 0xE3, 0xA6, 0x0B, 0xAB, 0x66,
    0xD7, 0xF3, 0xDB, 0xBB, 0x45, 0x79, 0xB3, 0x80, 0xF9, 0x57, 0x5D, 0xDB,
    0x14, 0x74, 0x79, 0x97, 0xA7, 0x58, 0x71, 0xF1, 0xDC, 0x1E, 0xCF, 0x43,
    0x56, 0x96, 0x9A, 0x55, 0x87, 0xF2, 0xA3, 0x22, 0x6F, 0x38, 0x49, 0x26,
    0x4D, 0x96, 0x93, 0x28, 0x58, 0x67, 0xD6, 0xA8, 0xFE, 0x08, 0xB5, 0x36,
    0xA8, 0x1F, 0x2D, 0xAB, 0xAE, 0x70, 0x5A, 0xB3, 0x57, 0xE5, 0xF2, 0xC1,
    0xD6, 0x94, 0x3F, 0xE7, 0xDB, 0x2D, 0x76, 0x4A, 0x83, 0x0C, 0xA7, 0x8A,
    0x96, 0x0F, 0xB8, 0x69, 0xAC, 0xB1, 0xE8, 0xA0, 0xCE, 0xA1, 0xB6, 0x4A,
    0xBA, 0xF1, 0x3F, 0x61, 0x4D, 0xF6, 0x7A, 0x26, 0x64, 0x48, 0x05, 0x0E,
    0xD0, 0xD7, 0x59, 0x06, 0x7F, 0xE7, 0x6A, 0x84, 0x3B, 0x3D, 0x1C, 0xD8,
    0x2B, 0xB0, 0xA2, 0x8B, 0xA6, 0x50, 0x66, 0x69, 0xBA, 0xF1, 0x5E, 0x15,
    0x02, 0x67, 0x6A, 0xAA, 0xA3, 0x59, 0x7A, 0x0F, 0x62, 0xC3, 0x6A, 0x02,
    0x73, 0x47, 0xA8, 0x30, 0x95, 0x22, 0xF6, 0x28, 0xDB, 0x62, 0x83, 0xC1,
    0x2E, 0x8C, 0x33, 0x18, 0xA0, 0x51, 0xFC, 0xAD, 0x5C, 0x6A, 0xA4, 0xA6,
    0x0B, 0x80, 0x4F, 0x19, 0x77, 0x47, 0xDA, 0x89, 0x16, 0xBF, 0x4E, 0x99,
    0xF6, 0x5A, 0xBC, 0xD0, 0x9A, 0x25, 0x1E, 0x74, 0x9A, 0xC5, 0xB6, 0xD6,
    0x6B, 0x87, 0x9A, 0x2F, 0x2B, 0x76, 0x80, 0x10, 0xD9, 0x29, 0xC1, 0x7B,
    0x7D, 0x7A, 0x39, 0x67, 0xA5, 0x6F, 0x66, 0x45, 0xF5, 0x7B, 0x91, 0xE1,
    0x14, 0x59, 0xF8, 0x17, 0x3B, 0x42, 0x52, 0x65, 0x29, 0x2E, 0x74, 0xD9,
    0xAB, 0xED, 0xA2, 0x1B, 0xDD, 0x83, 0x59, 0x32, 0x87, 0xCC, 0xCC, 0xC3,
    0x5F, 0x34, 0xF3, 0x85, 0xFC, 0x0F, 0x1E, 0x76, 0x04, 0xA0, 0x9F, 0x6B,
    0x12, 0x95, 0x1A, 0x43, 0x93, 0xE2, 0x17, 0x7D, 0xFE, 0xB7, 0xCD };

void LoginSession::initMessages()
{
    using namespace config;
    unsigned char data[] = {
	1, 1, (unsigned char)ipaddr[3], (unsigned char)ipaddr[2],
	(unsigned char)ipaddr[1], (unsigned char)ipaddr[0],
	(unsigned char)((loginPort >> 8) & 0xFF),
	(unsigned char)(loginPort & 0xFF), 'R', 'e', 'n', 'a', 'i', 's', 's',
	'a', 'n', 'c', 'e'
    };
    slistCrc = getCrc32(data, sizeof(data));

    unsigned char *sname = new unsigned char[1024];
    slistLen = compress(data, sname, sizeof(data), 1024);
    if (slistLen < 0) {
	LOG4CPLUS_FATAL(log, "Failed to compress server list information.");
	abort();
    }
    srvNamelist = sname;
}

LoginSession::LoginSession(int fd, in_addr_t ip, int now) :
    BasicSession(fd, now, ip), service(new EncryptionService()), ordinal(0)
{
    // Send welcome message
    char data[] = { 0x1b, 'C', 'O', 'N', 'N', 'E', 'C', 'T', 'E', 'D', ' ', 'S',
        'E', 'R', 'V', 'E', 'R', 0x0a };

    DAPacket p(Server::WELCOME, data, sizeof(data));
    p.writeData(fd);
}

LoginSession::~LoginSession()
{
    delete service;
}

// Generated as a result of a read being ready on the file descriptor
void LoginSession::dataReady()
{
    DAPacket *p;
    try {
        p = new DAPacket(fd);
    }
    catch (DAPacket::PacketError &pe) {
        LOG4CPLUS_TRACE(log, "Packet error while reading from client "
			<< getIpString());
        open = false;
        return;
    }

    try {
        switch (p->getCode()) {
        case (Client::START_ENCRYPT):
            if (checkVersion(p))
                startEncrypt();
            else
                open = false;
            break;
        case (Client::CONFIRM_ENCRYPT):
            confirmEncrypt(p);
            break;
        case (Client::IGNORE_0):
            break;
        case (Client::CREATE_CHARACTER):
            createCharacter(p);
            break;
        case (Client::SET_ATTRIBUTES):
            setAttributes(p);
            break;
        case (Client::CHANGE_PASSWORD):
            changePassword(p);
            break;
        case (Client::QUERY_HOMEPAGE):
            queryHomepage();
            break;
        case (Client::LOGIN):
            login(p);
            break;
        case (Client::PING):
            break;
        case (Client::ATTACH_CHARACTER):
            loggedout(p);
            break;
        case (Client::LOGOUT): //TODO rename this as client exited
            open = false;
            break;
        case (Client::QUERY_MOTD):
            getMotd();
            break;
        default:
            //Unrecognised packet code
            //open = false;
            break;
        }
    }

    catch (DAPacket::PacketError &pe) {
        // Something went wrong while reading a packet!
        LOG4CPLUS_TRACE(log, "Packet error while reading from client "
			<< getIpString());
        open = false;
    }
    catch (SessionError &se) {
        //tried to encrypt/decrypt something without using keytable, probably a bad packet code
        LOG4CPLUS_TRACE(log, "Encryption error while reading from client "
			<< getIpString());
        open = false;
    }

    delete p;
}

void LoginSession::loggedout(DAPacket *p)
{
    unsigned char seed = p->extractByte();
    if (seed > 9) {
        open = false;
        return;
    }

    std::string key(p->extractString());
    std::string name(p->extractString());

    if (name == "" || key.length() == 0) {
        open = false;
        return;
    }

    delete service;
    service = new EncryptionService(name.c_str(), key.c_str(), key.length(),
        seed);

    char ord = ordinal++;
    DAPacket reply(Server::GET_MOTD, &ord, 1);
    reply.appendBytes(sizeof(motd_crc), (const char *) motd_crc);
    service->encrypt(&reply);
    reply.writeData(fd);
}

//Returns true if the client version matches the version in defines.h
bool LoginSession::checkVersion(DAPacket *p)
{
    unsigned char hi = p->extractByte();
    unsigned char lo = p->extractByte();
    unsigned short ver = (hi << 8) + lo;
    return ver >= CLIENT_VERSION_MIN;
}

void LoginSession::startEncrypt()
{
    std::unique_ptr<DAPacket> reply = service->getSetPacket(slistCrc);
    reply->writeData(fd);
}

void LoginSession::getMotd()
{
    char ord = ordinal++;
    DAPacket reply(Server::GET_MOTD, &ord, 1);
    reply.appendBytes(sizeof(motd), (const char *) motd);
    service->encrypt(&reply);
    reply.writeData(fd);
}

void LoginSession::confirmEncrypt(DAPacket *p)
{
    service->decrypt(p);

    bool sendList = p->extractByte();
    if (sendList) {
	char ord = ordinal++;
	DAPacket reply(Server::UPDATE_SERVERLIST, &ord, 1);
	reply.appendShort(slistLen);
	reply.appendBytes(slistLen, (const char *)srvNamelist);
	service->encrypt(&reply);
	reply.writeData(fd);
    }

    else {
	char ord = ordinal++;
	DAPacket reply(Server::GET_MOTD, &ord, 1);
	reply.appendBytes(sizeof(motd_crc), (const char *) motd_crc);
	service->encrypt(&reply);
	reply.writeData(fd);
    }
}

void LoginSession::queryHomepage()
{
    unsigned char repData[2] = { ordinal++, 0x03 };
    DAPacket response(Server::SEND_HOMEPAGE, (char *) repData, sizeof(repData));
    response.appendString(0x17, "http://www.darkages.com");
    service->encrypt(&response);
    response.writeData(fd);
}

void LoginSession::createCharacter(DAPacket *p)
{
    service->decrypt(p);

    std::string name(p->extractString());
    std::string pw(p->extractString());
    if (name.length() > 12 || pw.length() > 8) {
        open = false;
    }

    // Remaining data: (size "none") none 0
    // probably safe to ignore

    // Request data manager to reserve name given
    if (!DataService::getService()->makeCharacter(name, pw.c_str(), clientIp)) {
        // TODO could be other causes of failure
        unsigned char ret[] = { ordinal++, 4 };
        DAPacket reply(Server::SERVER_MESSAGE, (char *) ret, 2);
        char badName[] =
            "Sorry, the name entered is not available, or the password is too short.";
        reply.appendString(sizeof(badName) - 1, badName);
        reply.appendByte('\0');
        service->encrypt(&reply);
        reply.writeData(fd);
    }
    else {
        //Character file created
        LOG4CPLUS_INFO(log, "Character " << name << " created by client "
		       << getIpString());
        editName = name;
        unsigned char ret[5] = { ordinal++, 0, 0, 0, 0 };
        DAPacket reply(Server::SERVER_MESSAGE, (char *) ret, 5);
        service->encrypt(&reply);
        reply.writeData(fd);
    }
}

void LoginSession::setAttributes(DAPacket *p)
{
    if (editName == "" || p->getDataLen() < 4) {
        open = false;
        return;
    }

    service->decrypt(p);
    char *data = p->getDataPtr();
    //Test gender bytes
    char gender = data[1] == 1 ? 'm' : 'f';
    if (!DataService::getService()->setAttributes(editName, data[0], data[2],
        gender)) {
        open = false;
        return;
    }

    unsigned char retData[3] = { ordinal++, 0, 0 };
    DAPacket reply(Server::SERVER_MESSAGE, (char *) retData, 3);
    service->encrypt(&reply);
    reply.writeData(fd);
    editName = "";
}

void LoginSession::changePassword(DAPacket *p)
{
    service->decrypt(p);
    std::string name(p->extractString());
    std::string oldPw(p->extractString());
    std::string newPw(p->extractString());

    if (DataService::getService()->changePassword(name, oldPw.c_str(),
        newPw.c_str())) {
        LOG4CPLUS_TRACE(log, "Password change accepted for character "
			<< name << ", requested by client "
			<< getIpString());
        //Success
        unsigned char retData[3] = { ordinal++, 0, 0 };
        DAPacket reply(Server::SERVER_MESSAGE, (char *) retData, 3);
        service->encrypt(&reply);
        reply.writeData(fd);
    }
    else {
        LOG4CPLUS_TRACE(log, "Password change rejected for character "
			<< name << ", requested by client "
			<< getIpString());
        unsigned char retData[] = { ordinal++, 0xf };
        DAPacket reply(Server::SERVER_MESSAGE, (char *) retData, 2);
        char failed[] = "Invalid username/password combination.";
        reply.appendString(sizeof(failed), failed);
        service->encrypt(&reply);
        reply.writeData(fd);
    }
}

void LoginSession::login(DAPacket *p)
{
    service->decrypt(p);
    using config::ipaddr;
    using config::gamePort;

    std::string name(p->extractString());
    std::string pw(p->extractString());
    int err;

    if (!(err = DataService::getService()->prepareLogin(name, pw.c_str(),
        clientIp))) {
	LOG4CPLUS_INFO(log, "Accepted login request for character "
		       << name << ", from client " << getIpString());
        unsigned char retData[4] = { ordinal++, 0, 0, 0 };
        DAPacket replyOne(Server::SERVER_MESSAGE, (char *) retData, 4);
        service->encrypt(&replyOne);
        replyOne.writeData(fd);
        //Send redirect packet
        char redirectData[] = { ipaddr[3], ipaddr[2], ipaddr[1], ipaddr[0] };
        DAPacket replyTwo(Server::REDIRECT, redirectData, sizeof(redirectData));
        replyTwo.appendShort(gamePort);
        replyTwo.appendByte(name.length() + service->getKeyLen() + 3);
        replyTwo.appendByte(service->getSeed());
        replyTwo.appendString(service->getKeyLen(), service->getKey());
        replyTwo.appendString(name.length(), name.c_str());
        replyTwo.appendString(0, 0);
        replyTwo.appendInt(0x2f100);
        replyTwo.writeData(fd);
    }
    else {
	LOG4CPLUS_INFO(log, "Failed login attempt for character " <<
		       name << ", requested by client " << getIpString());
        unsigned char failed[] = { ordinal++, 0xf };
        DAPacket reply(Server::SERVER_MESSAGE, (char *) failed, 2);
        if (err == E_LOGGEDIN) {
            char rep[] = "You are already logged in.";
            reply.appendString(sizeof(rep) - 1, rep);
        }
        else if (err == E_NOEXIST) {
            char rep[] = "This character does not exist.";
            reply.appendString(sizeof(rep) - 1, rep);
        }
        else if (err == E_WRONGPW) {
            char rep[] = "Incorrect password.";
            reply.appendString(sizeof(rep) - 1, rep);
        }
        else if (err == E_INVALID) {
            char rep[] = "Invalid username/password combination.";
            reply.appendString(sizeof(rep) - 1, rep);
        }
        service->encrypt(&reply);
        reply.writeData(fd);
    }

}
