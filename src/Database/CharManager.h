/*
 * CharManager.h
 *
 *  Created on: 2013-06-30
 *      Author: per
 */

#ifndef CHARMANAGER_H_
#define CHARMANAGER_H_

#include <vector>
#include <mutex>
#ifdef WIN32
#include <libpq-fe.h>
#define snprintf _snprintf
#define PATH_MAX MAX_PATH
#else
#include <postgresql/libpq-fe.h>
#endif
#include <pthread.h>
#include <unordered_map>

class Character;
class CharacterSession;

namespace Database
{

struct CharStats
{
    unsigned int map;
    unsigned short x;
    unsigned short y;
    unsigned int hp;
    unsigned int mp;
    unsigned int maxHp;
    unsigned int maxMp;
    unsigned short strength;
    unsigned short int_;
    unsigned short wis;
    unsigned short con;
    unsigned short dex;
    unsigned short stPts;
    unsigned short lev;
    unsigned short ab;
    unsigned short dead;
    long long exp;
    long long ap;
    int id;
    unsigned int gold;
    unsigned short labor;
    unsigned int laborReset;
    unsigned int lastLogin;
    unsigned int storedGold;
    unsigned int exchangeGold;
    int settings;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%u\t%hu\t%hu\t%u\t%u\t"
            "%u\t%u\t%hu\t%hu\t%hu\t"
            "%hu\t%hu\t%hu\t%hu\t%hu\t"
            "%hu\t%lld\t%lld\t%d\t%u\t"
            "%hu\t%u\t%u\t%u\t%u\t"
            "%d\n", map, x, y, hp, mp, maxHp, maxMp, strength, int_, wis, dex,
            con, stPts, lev, ab, dead, exp, ap, id, gold, labor, laborReset,
            lastLogin, storedGold, exchangeGold, settings);
    }
};

struct InsertEquip
{
    int charId, itemId;
    unsigned short slot, qty, mod, identified;
    unsigned dur;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%d\t%d\t%hu\t%hu\t%u\t%hu\t%hu\n", charId,
            itemId, slot, qty, dur, mod, identified);
    }
};

struct AccidentItem
{
    int charId, itemId;
    unsigned short qty, mod;
    unsigned dur;
    bool temp;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%d\t%d\t%hu\t%u\t%hu\t%s\n", charId,
            itemId, qty, dur, mod, temp ? "TRUE" : "FALSE");
    }
};

struct UpdateSkill
{
    unsigned short level, slot;
    unsigned uses, cd;
    int charId;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%d\t%hu\t%hu\t%u\t%u\n", charId, level,
            slot, uses, cd);
    }
};

struct TrackerEntry
{
    int charId, mobId, count, questId;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%d\t%d\t%d\t%d\n", charId, questId, mobId,
            count);
    }
};

struct Effect
{
    int charId, buffId;
    unsigned duration;

    int copyString(char *buffer, int len)
    {
        return snprintf(buffer, len, "%d\t%d\t%u\n", charId, buffId, duration);
    }
};

class CharManager
{
public:
    CharManager(const char *connParams);
    ~CharManager();

    //Load/save functions
    void finish();
    void freeCharacter(Character *c);
    Character *getCharacter(const char *lname, CharacterSession *who);
    void saveCharacter(Character *c);

    //Update functions----------------------------------------------------
    //Storage
    void storageQtyChange(int charId, int itemId, short mod, int newQty);
    void storageAddItem(int charId, int itemId, short mod, int qty);
    void storageRemoveItem(int charId, int itemId, short mod);
    //Inventory
    void removeItem(int charId, unsigned short slot);
    void addItem(int charId, int itemId, unsigned short slot, short mod,
        short qty, int dur, short idd);
    void updateItem(int charId, unsigned short slot, short mod, short qty,
        int dur, short idd);
    //Quests
    void addQuest(int charId, int qid, int qp, int timer);
    void updateQuest(int charId, int qid, int qp, int timer);
    //Legend marks
    void addMark(int charId, int markId, const char *textParam, int intParam,
        int timestamp);
    void updateMark(int charId, int markId, const char *textParam, int intParam,
        int timestamp);
    void deleteMark(int charId, int markId);
    //Skills
    void addSkill(int charId, int skillId, unsigned short slot);
    void removeSkill(int charId, unsigned short slot);
    void moveSkill(int charId, unsigned short oldSlot, unsigned short newSlot);
    void swapSkills(int charId, unsigned short slotOne, unsigned short slotTwo,
        int idOne, int idTwo);
    //Secrets
    void addSecret(int charId, int skillId, unsigned short slot);
    void removeSecret(int charId, unsigned short slot);
    void moveSecret(int charId, unsigned short oldSlot, unsigned short newSlot);
    void swapSecrets(int charId, unsigned short slotOne, unsigned short slotTwo,
        int idOne, int idTwo);
    //Attributes
    void updatePath(int charId, unsigned short path, unsigned short pathmask);
    void updateHairstyle(int charId, unsigned short style);
    void updateHaircolor(int charId, unsigned short color);
    void updateNation(int charId, unsigned short nation);
    //--------------------------------------------------------------------

    void lock();
    void unlock();

    void ban(const char *name, int exp);

private:
    std::vector<Character *> outgoing;
    PGconn *conn, *bgconn;

    std::vector<CharStats> chars;
    std::vector<InsertEquip> equips;
    std::vector<AccidentItem> items;
    std::vector<UpdateSkill> skills;
    std::vector<UpdateSkill> secrets;
    std::vector<Effect> effects;
    std::vector<TrackerEntry> trackers;
    std::unordered_map<std::string, int> banned;

    pthread_t worker;
    std::mutex cmlock;
    bool canJoin;
};

} /* namespace Database */
#endif /* CHARMANAGER_H_ */
