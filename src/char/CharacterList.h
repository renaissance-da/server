/*
 * CharacterList.h
 *
 *  Created on: 2011-09-07
 *      Author: per
 */

#ifndef CHARACTERLIST_H_
#define CHARACTERLIST_H_

#include <map>
#include <string>
#include <vector>
#include "Character.h"
#include "CharacterSession.h"

class CharacterList: public std::map<std::string, CharacterSession *>
{
public:
    CharacterList(std::string dirPath);
    virtual ~CharacterList();bool makeCharacter(std::string name,
        char const *pw, uint32_t ipaddr);bool setAttributes(std::string name,
        unsigned char hair, unsigned char haircolor, char gender);bool changePassword(
        std::string name, char const *pwOld, char const *pwNew);
    int prepareLogin(std::string name, char const *pw, unsigned int ip);
    std::vector<Character *> *countryList();

    Character *getCharacter(std::string name, CharacterSession *who,
        unsigned int ip);
    CharacterSession *findSession(Character *who);

    void freeCharacter(Character *c);
    void foreach(int (*action)(void *, void *), void *param);bool validateName(
        std::string &name);bool validatePw(const char *pw);
    std::map<std::string, CharacterSession *>::iterator find(std::string key);

private:

    struct LoginWaiting
    {
        unsigned int ip;
        unsigned int timestamp;
        //TODO Could use encryption key as well
    };

    std::map<std::string, LoginWaiting> loginWaits;
    std::string characters;
    std::map<uint32_t, int> lastCreate;
};

#endif /* CHARACTERLIST_H_ */
