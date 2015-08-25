/*
 * CharacterList.cpp
 *
 *  Created on: 2011-09-07
 *      Author: per
 */

#include "CharacterList.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Hash.h"
#include <string.h>
#include <time.h>
#include "defines.h"
#include "DataLoaders.h"
#include <algorithm>
#include "lower.h"
#include "CharManager.h"
#include "core.h"
#include "log4cplus/loggingmacros.h"

CharacterList::CharacterList(std::string dirPath)
{
    characters = dirPath;
}

CharacterList::~CharacterList()
{
}

bool CharacterList::validateName(std::string &name)
{
    if (name.length() < 4 || name.length() > 12)
        return false;

    for (auto it = name.begin(); it != name.end(); it++) {
        if (!isalpha(*it))
            return false;
    }
    return true;
}

std::map<std::string, CharacterSession *>::iterator CharacterList::find(
    std::string key)
{
    lower(key);
    return std::map<std::string, CharacterSession *>::find(key);
}

bool CharacterList::validatePw(const char *pw)
{
    size_t pwlen = strlen(pw);
    if (pwlen < 4 || pwlen > 12)
        return false;
    for (size_t i = 0; i < pwlen; i++) {
        if (!isalnum(pw[i]))
            return false;
    }

    return true;
}

bool CharacterList::makeCharacter(std::string name, char const *pw,
    in_addr_t ipaddr)
{
    // Check time restriction
    int timeLimit = time(NULL) - 60;
    auto it = lastCreate.find(ipaddr);
    if (it != lastCreate.end() && it->second > timeLimit)
        return false;

    if (!validatePw(pw))
        return false;
    if (!validateName(name))
        return false;

    bool r = Database::makeCharacter(name.c_str(), pw);
    if (r)
        lastCreate[ipaddr] = time(NULL);
    return r;
}

bool CharacterList::setAttributes(std::string name, unsigned char hair,
    unsigned char haircolor, char gender)
{

    //TODO check max hairstyle/color on client
    if (hair > 18 || haircolor > 14 || (gender != 'f' && gender != 'm'))
        return false;

    lower(name);
    return Database::setAttributes(name.c_str(), hair, haircolor, gender);
}

bool CharacterList::changePassword(std::string name, char const *pwOld,
    char const *pwNew)
{
    if (!validatePw(pwNew))
        return false;

    lower(name);
    return Database::changePassword(name.c_str(), pwOld, pwNew);
}

int CharacterList::prepareLogin(std::string name, char const *pw,
    unsigned int ip)
{
    if (!validatePw(pw) || !validateName(name))
        return E_INVALID;

    lower(name);

    std::map<std::string, CharacterSession*>::iterator online_c = find(name);
    if (online_c != end()) {
        //Character is already logged in
        //Disconnect the previous login
        //I think its possible to delete the session without removing from here
        online_c->second->disconnect();

        return E_LOGGEDIN;
    }
    if (loginWaits.count(name)) {
        if (time(NULL) - loginWaits[name].timestamp > 5)
            loginWaits.erase(name);
        else {
            return E_LOGGEDIN;
            loginWaits.erase(name);//the other player can't log in either
        }
        //Two logins requested at ~same time
        //Reject this one only
    }

    int r = Database::testPassword(name.c_str(), pw);
    if (r)
        return r;

    LoginWaiting w;
    w.ip = ip;
    w.timestamp = time(NULL);
    loginWaits[name] = w;

    return 0;
}

Character *CharacterList::getCharacter(std::string name, CharacterSession *who,
    unsigned int ip)
{
    std::string lname = name;
    lower(lname);

    if (!loginWaits.count(lname))
        return 0;

    loginWaits.erase(lname);
    Database::cm->lock();
    Character *ret = Database::cm->getCharacter(lname.c_str(), who);
    Database::cm->unlock();
    if (!ret)
        return 0;

    (*this)[lname] = who;
    ret->changeName(name);
    LOG4CPLUS_INFO(core::log, name << " has logged in.");

    return ret;
}

void CharacterList::freeCharacter(Character *c)
{
    c->cancelTrade();
    Database::cm->lock();
    Database::cm->freeCharacter(c);
    Database::cm->unlock();

    std::string name = c->getName();
    LOG4CPLUS_INFO(core::log, name << " has logged out.");

    lower(name);
    assert(erase(name));
}

void CharacterList::foreach(int (*action)(void *, void *), void *param)
{
    for (CharacterList::iterator it = begin(); it != end(); it++) {
        action((void *) it->second, param);
    }
}

CharacterSession *CharacterList::findSession(Character *c)
{
    return c->getSession();
}

std::vector<Character *> *CharacterList::countryList()
{
    //TODO implement faster method, and do sorting
    std::vector<Character *> *ret = new std::vector<Character *>;
    std::for_each(begin(), end(),
        [&](std::pair<std::string, CharacterSession *> cs) {
            ret->push_back(cs.second->getCharacter());
        });

    return ret;
}
