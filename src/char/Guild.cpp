/*
 * Guild.cpp
 *
 *  Created on: Sep 20, 2014
 *      Author: per
 */

#include <Guild.h>
#include "lower.h"
#include "Character.h"

std::unordered_map<int, Guild *> Guild::guilds;

Guild::Guild(int id, const char *name) :
    id(id), name(name)
{
    guilds[id] = this;
}

Guild::~Guild()
{
    for (Character *c : onlineMembers) {
        c->setGuild(0);
    }
    guilds.erase(id);
}

void Guild::addMember(Character *who, Guild::Rank rank, bool isNew)
{
    if (isNew)
        memberList.push_back( { who->getName(), rank });
    onlineMembers.push_back(who);
    who->setGuild(this);
    who->setRank(rank);
}

/**
 * Add a name to the roll of guild members.
 * \param[in] The name to be added.
 */
void Guild::addToRoll(const char *name, Guild::Rank rank)
{
    memberList.push_back( { name, rank });
}

/**
 * Remove a member from this guild. The member does not need
 * to be online.
 * \return True if a member was removed, false otherwise
 */
bool Guild::removeMember(std::string name)
{
    bool ret = false;
    lower(name);

    //remove from list
    for (auto it = memberList.begin(); it != memberList.end(); it++) {
        std::string cname = it->first;
        lower(cname);
        if (cname == name) {
            memberList.erase(it);
            //erase from db
            ret = true;
            break;
        }
    }

    //is online?
    for (auto it = onlineMembers.begin(); it != onlineMembers.end(); it++) {
        std::string cname = (*it)->getName();
        lower(cname);
        if (cname == name) {
            (*it)->setGuild(0);
            onlineMembers.erase(it);
            break;
        }
    }

    return ret;
}

/**
 * Update a guild member's rank. The member does not need
 * to be online.
 * \return True if the rank was updated, false otherwise
 */
bool Guild::setRank(std::string name, Rank newRank)
{
    bool ret = false;
    lower(name);

    //update on list
    for (auto it = memberList.begin(); it != memberList.end(); it++) {
        std::string cname = it->first;
        lower(cname);
        if (cname == name) {
            it->second = newRank;
            ret = true;
            break;
        }
    }

    //is online?
    for (auto it = onlineMembers.begin(); it != onlineMembers.end(); it++) {
        std::string cname = (*it)->getName();
        lower(cname);
        if (cname == name) {
            //(*it)->setGuild(0);
            (*it)->setRank(newRank);
            break;
        }
    }

    return ret;
}

/**
 * Get a guild by its unique ID.
 * \param[in] id The ID of the guild to look up.
 * \return A pointer to the guild with the given ID, or null.
 */
Guild *Guild::getById(int id)
{
    auto it = guilds.find(id);

    if (it != guilds.end())
        return it->second;
    else
        return 0;
}

/**
 * Get a string describing the value of rank given.
 * Currently the only ranks are blank, Council and Leader, but more
 * could be added.
 * \param[in] rank The rank to find the string description for.
 * \return A pointer to a string describing the rank.
 */
const char *Guild::getRankName(Guild::Rank rank)
{
    switch (rank) {
    case Guild::COUNCIL:
        return "Council";
    case Guild::LEADER:
        return "Leader";
    default:
        return "";
    }
}

/**
 * Get the rank of a member of this guild.
 * \param[in] name The name of the member to look up.
 * \return The named member's ranking in this guild. If they are not a member,
 * returns Guild::NON_MEMBER.
 */
Guild::Rank Guild::getRank(const char *name)
{
    std::string lname(name);
    lower(lname);
    for (auto it = memberList.begin(); it != memberList.end(); it++) {
        std::string cname(it->first);
        if (lname == cname)
            return it->second;
    }

    return Guild::NON_MEMBER;
}
