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
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

log4cplus::Logger LoginSession::log = log4cplus::Logger::getInstance(
    "renaissance.login");

const unsigned char *srvNamelist;
int slistCrc = 0;
int slistLen;
char *motd;
int motdLen, motd_crc;

void LoginSession::initMessages()
{
    using namespace config;
    unsigned char data[] = {
	1, 1, (unsigned char)ipaddr[3], (unsigned char)ipaddr[2],
	(unsigned char)ipaddr[1], (unsigned char)ipaddr[0],
	(unsigned char)((loginPort >> 8) & 0xFF),
	(unsigned char)(loginPort & 0xFF), 'R', 'e', 'n', 'a', 'i', 's', 's',
	'a', 'n', 'c', 'e', '\0'
    };
    slistCrc = getCrc32(data, sizeof(data));

    unsigned char *sname = new unsigned char[1024];
    slistLen = compress(data, sname, sizeof(data), 1024);
    if (slistLen < 0) {
	LOG4CPLUS_FATAL(log, "Failed to compress server list information.");
	abort();
    }
    srvNamelist = sname;

    // Read MOTD
    std::fstream motd_file;
    motd_file.open(motd_filename, std::ios::in | std::ios::binary);
    if (!motd_file.is_open()) {
	LOG4CPLUS_FATAL(log, "Failed to load message of the day.");
	abort();
    }
    struct stat st;
    if (stat(motd_filename, &st) == -1) {
	LOG4CPLUS_FATAL(log, "Couldn't stat motd file.");
	abort();
    }
    char *motdUncompressed = new char[st.st_size];
    motd_file.read(motdUncompressed, st.st_size);
    motdLen = std::max((off_t)1024, st.st_size);
    ::motd = new char[motdLen];

    motdLen = compress((unsigned char *)motdUncompressed,
		       (unsigned char *)::motd,
		       st.st_size,
		       motdLen);
    delete[] motdUncompressed;
}

LoginSession::LoginSession(int fd, in_addr_t ip, int now) :
    BasicSession(fd, now, ip), service(new EncryptionService()), ordinal(0)
{
    LOG4CPLUS_TRACE(log, "Accepted connection from " << getIpString());
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
    reply.appendByte(0);
    reply.appendInt(motd_crc);
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
    reply.appendByte(1);
    reply.appendShort(motdLen);
    reply.appendBytes(motdLen, motd);
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
	reply.appendByte(0);
	reply.appendInt(motd_crc);
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
