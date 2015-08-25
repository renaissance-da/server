/*
 * Guild.h
 *
 *  Created on: Sep 20, 2014
 *      Author: per
 */

#ifndef GUILD_H_
#define GUILD_H_

#include <string>
#include <vector>
#include <unordered_map>

class Character;

class Guild
{
public:
    enum Rank
    {
        MEMBER = 0, COUNCIL = 1, LEADER = 2, NON_MEMBER = 3
    };

    Guild(int id, const char *name);
    ~Guild();

    int getId()
    {
        return id;
    }
    std::string getName()
    {
        return name;
    }

    void addMember(Character *who, Rank rank, bool isNew);
    bool removeMember(std::string name);
    void addToRoll(const char *name, Rank rank);
    bool setRank(std::string name, Rank newRank);

    const std::vector<Character *> &getOnline()
    {
        return onlineMembers;
    }
    const std::vector<std::pair<std::string, Rank> > getMemberList()
    {
        return memberList;
    }

    static Guild *getById(int id);
    const char *getRankName(Guild::Rank rank);
    Guild::Rank getRank(const char *name);

private:
    int id;
    std::string name;
    std::vector<Character *> onlineMembers;
    std::vector<std::pair<std::string, Rank> > memberList;

    static std::unordered_map<int, Guild *> guilds;
};

#endif /* GUILD_H_ */
