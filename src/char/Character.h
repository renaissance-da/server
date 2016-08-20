/*
 * Character.h
 *
 *  Created on: 2011-09-07
 *      Author: per
 */

#ifndef CHARACTER_H_
#define CHARACTER_H_

#include <string>
#include "Stats.h"
#include "Skill.h"
#include "Entity.h"
#include "IDataStream.h"
#include <vector>
#include <list>
#include "Paths.h"
#include "defines.h"
#include "Equipment.h"
#include "Field.h"
#include "Secret.h"
#include "Exchange.h"
#include <assert.h>
#include <map>
#include <functional>
#include "Inventory.h"
#include "Tracker.h"
#include "Legend.h"
#include "Group.h"
#include "Guild.h"
#include "log4cplus/logger.h"

class CharacterSession;
class Guild;

class Character: public Entity
{
public:

    struct LegendItem
    {
        Legend *base;
        char text[128];
        char textParam[128];
        int intParam, timestamp, textlen;
    };

    struct StorageItem
    {
        int id;
        short mod;
        int qty;
    };

    struct QuestProgress
    {
        int progress;
        int timer;
    };

    enum StatFlags
    {
        FLAG_SECONDARY = 0x04,
        FLAG_POINTS = 0x08,
        FLAG_HPMP = 0x10,
        FLAG_MAX = 0x20,
        FLAG_ALL = 0x3C
    };

    enum Settings
    {
        LISTEN_TO_WHISPERS = 1,
        JOIN_A_GROUP = 2,
        LISTEN_TO_SHOUT = 4,
        BELIEVE_IN_WISDOM = 8,
        BELIEVE_IN_MAGIC = 0x10,
        EXCHANGE = 0x20,
        GROUP_WINDOW = 0x40,
        CLAN_WHISPER = 0x80
    };

    Character(int id, const char *name, Map *map, unsigned short x,
        unsigned short y, int hp, int mp, int maxHp, int maxMp, short strength,
        short int_, short wis, short dex, short con, unsigned char hairstyle,
        unsigned char haircolor, unsigned char statPoints, char gender,
        short path, int pathmask, short level, short ab, long long exp,
        long long ap, unsigned int gold, unsigned int storedGold,
        unsigned short priv,
        bool dead, unsigned short labor, unsigned int laborReset,
        unsigned short nation, int settings);
    virtual ~Character();

    void getStatBlock(IDataStream *dp, unsigned int flags);
    void getViewedBlock(IDataStream *op);
    void getLegendInfo(IDataStream *dp);
    void getProfileInfo(IDataStream *dp);
    int getTni();bool tryMove(char dir);
    void gainExp(unsigned int exp);
    void gainExp(unsigned int exp, int mobId);
    void damage(Entity *a, int amt);
    void heal(float amt, bool show = true);
    void gearStruck();
    void weaponStruck();
    void levelUp();
    const Stats *getStats();
    Element getAtkEle();
    Element getDefEle();
    void changeName(std::string name)
    {
        this->name = name;
    }

    Skill **getSkills()
    {
        return skills;
    }
    Secret **getSecrets()
    {
        return secrets;
    }
    bool useSkill(char slot);
    void attack();bool tick(std::vector<std::function<void()> > &deferred)
        override;
    void clickGround(unsigned short x, unsigned short y);
    void clickEntity(int oid);

    void setSession(CharacterSession *s)
    {
        session = s;
    }
    CharacterSession *getSession()
    {
        return session;
    }
    virtual EntityType getType()
    {
        return E_CHARACTER;
    }
    virtual int getLevel()
    {
        return level;
    }
    void setHairstyle(unsigned char style);
    void setHaircolor(unsigned char color);
    unsigned char getHairstyle()
    {
        return hairstyle;
    }
    unsigned char getHaircolor()
    {
        return haircolor;
    }
    unsigned short getPrivilegeLevel()
    {
        return priv;
    }
    unsigned short effectiveLevel()
    {
        return level + (isMaster() ? 1 : 0) + ab;
    }

    void swapInv(char dst, char src);
    void swapSkills(char dst, char src);
    void swapSecrets(char dst, char src);
    void incStat(char which);
    void incStr(int amt);
    void incDex(int amt);
    void incCon(int amt);
    void incWis(int amt);
    void incInt(int amt);
    void addSkill(int id, short level, short slot, int uses, unsigned cd = 0);
    void addSecret(int id, short level, short slot, int uses, unsigned cd = 0);
    char learnSkill(Skill *sk);
    char learnSecret(Secret *sp);
    void addLegendItem(int id, const char *textParam, int intParam);
    void addLegendItem(int id, const char *textParam, int intParam,
        int timestamp);
    std::vector<LegendItem> &getLegend()
    {
        return legend;
    }
    void changeLegendQty(int id, int intParam);
    void changeLegendQty(int id, int intParam, const char *textParam);
    void removeLegend(int id);
    int getRegenTimer();
    int getMpRegenTimer();
    int getId()
    {
        return charId;
    }
    void regen();
    void mpRegen();
    int moveType()
    {
        return MOVE_SOLID;
    }
    bool takeExp(unsigned int amt);
    void addMaxHp(int amt);
    void addMaxMp(int amt);bool extendedViewType()
    {
        return true;
    }

    unsigned char getAb()
    {
        return ab;
    }
    long long getAp()
    {
        return ap;
    }
    const Stats *getBaseStats()
    {
        return &baseStats;
    }
    long long getExp()
    {
        return exp;
    }
    int getMp()
    {
        return curMp;
    }
    void addMp(int mp) override;
    Path getPath()
    {
        return isMaster() ? Master : path;
    }
    Path getBasePath()
    {
        return path;
    }
    bool isMaster()
    {
        return pathMask & paths[Master].mask;
    }
    void setMaster();
    int getPathMask()
    {
        return pathMask;
    }
    unsigned char getStatPoints()
    {
        return statPoints;
    }
    unsigned int getGold()
    {
        return gold;
    }
    unsigned short getDispHp();
    void useItem(char slot);bool getItem(Item *item);bool isUnarmed()
    {
        return !(equipment[Equipment::WEAPON] || equipment[Equipment::SHIELD]);
    }
    bool isCasting()
    {
        return chantSet && !ghost;
    }
    bool isDead()
    {
        return ghost;
    }
    void putItem(Item *item, unsigned char slot);
    void unequip(char slot);
    void dropItem(char slot, unsigned short x, unsigned short y,
        unsigned int num);
    void dropGold(unsigned int amt, unsigned short x, unsigned short y);
    void pickupItem(char slot, unsigned short x, unsigned short y);
    void addGold(int amt);bool removeSkill(int skid);
    Item **getEquipment()
    {
        return (Item **) equipment;
    }
    unsigned short countItems(int id, unsigned short mod);
    unsigned short getItemQty(unsigned short slot);
    void setPath(unsigned short p);
    int skillLevel(int id);bool hasSkill(int id)
    {
        return skillLevel(id) != -1;
    }
    bool hasSecret(int id)
    {
        return skillLevel(id) != -1;
    }
    void fieldJump(short mapId);
    unsigned int getMaxHp()
    {
        return stats.getHp();
    }
    bool canInvite()
    {
        return (settings & JOIN_A_GROUP) && !group;
    }
    Group *getGroup()
    {
        return group;
    }
    void setGroup(Group *g)
    {
        assert(!group);
        group = g;
    }
    void clearGroup()
    {
        group = 0;
    }
    StatusEffect *addStatusEffect(int skid);
    StatusEffect *addStatusEffect(int skid, int dur);bool removeEffect(
        StatusEffect::Kind kind);
    unsigned char getAtkAnim();
    void cancelCasting();
    void alive();
    short countFreeSlots();bool useLabor(unsigned short amt);
    void getLaborCountTime(unsigned short &count, unsigned int &ltime);
    int repairAllCost();bool repairAll();bool repair(unsigned int slot);
    void die();
    void setNationCoords();
    unsigned short getNation()
    {
        return nation;
    }
    void setNation(unsigned short n);
    void addTracker(int qid, int *mobList, int n, int *qty = 0);
    void deleteTracker(int qid);
    void addKill(int mobId, int nKills = 1);
    unsigned int countKills(int trackerId, int mobId);
    unsigned int countAllKills(int trackerId);
    void forgetSkill(int slot);
    void forgetSecret(int slot);
    void changedMap(Map *newmap);

    void rededicate(Path p);

    void cancelTrade();
    void confirmTrade();
    void giveItem(int oid, char slot);
    void giveGold(int oid, unsigned int amt);
    Item *removeItem(int slot);
    Item *removeItem(int slot, int amt);
    void lostItem(Item *itm)
    {
        lostItems.push_back(itm);
    }
    void startTrade(int oid);
    void tradeItem(int oid, char slot);
    void tradeItem(int oid, char slot, char amt);
    void tradeGold(int oid, int amt);
    unsigned int getExchangeGold();
    unsigned short curWgt()
    {
        return inventory.getWeight() + eqpWgt;
    }
    short getFreeWeight()
    {
        return maxWeight - curWgt();
    }
    bool canAct();
    StatusEffect **getEffects()
    {
        return effects;
    }
    std::vector<Item *> &getLostItems()
    {
        return lostItems;
    }

    void startChant(unsigned char count);bool useSecret(char slot, int oid);
    const char *getGender();
    void refresh();

    int getQuestProgress(int questId);
    void setQuestProgress(int questId, int progress);
    int getQuestTimer(int questId);
    void setQuestTimer(int questId, int timer);
    const std::map<int, QuestProgress> &getQuests()
    {
        return quests;
    }
    Inventory<MAX_EXCHANGE> *getExchangeItems()
    {
        return &exchange;
    }
    Inventory<NUM_ITEMS> &getInventory()
    {
        return inventory;
    }
    void finishExchange(Inventory<MAX_EXCHANGE> *exchange);
    void clearExchange()
    {
        exchange.clear();
    }
    void showStorage(IDataStream *dp);bool withdraw(int slot, int qty);bool deposit(
        Item *itm);bool depositGold(unsigned int amt)
    {
        if (amt > gold || storedGold + amt > MAX_GOLD)
            return false;
        storedGold += amt;
        addGold(-amt);
        return true;
    }
    bool withdrawGold(unsigned int amt)
    {
        if (amt > storedGold || gold + amt > MAX_GOLD)
            return false;
        storedGold -= amt;
        addGold(amt);
        return true;
    }
    unsigned int getStoredGold()
    {
        return storedGold;
    }
    void setStored(int itemId, int qty, short mod);
    const std::vector<StorageItem> &getStorage()
    {
        return storage;
    }
    const char *getLegendTextParam(int legendId);
    int getLegendTimestamp(int legendId);
    Guild *getGuild()
    {
        return guild;
    }
    void setGuild(Guild *g)
    {
        guild = g;
    }
    Guild::Rank getRank()
    {
        return rank;
    }
    void setRank(Guild::Rank rank)
    {
        this->rank = rank;
    }

    void addedItem(Item *item, int slot);
    void removedItem(int slot);
    void reload();
    void getSettings(IDataStream *dp, int which = 0);
    int getSettings()
    {
        return settings;
    }
    void toggleSetting(int which);

    //DisableUpdates template
    void disableUpdates()
    {
        updateDb = false;
    }
    void enableUpdates()
    {
        updateDb = true;
    }

    //Observation functions
    void showViewable(Viewable *v) override;
    void showViewables(const std::vector<Viewable *> &who) override;
    void talked(Entity *who, std::string text, unsigned char channel) override;
    void showAction(Viewable *who, char action, short duration) override;
    void entityStruck(Entity *who, unsigned short dispHp, unsigned char sound)
        override;
    void showEffect(Viewable *who, unsigned short effectId, int duration)
        override;
    void playSound(unsigned char sound) override;
    void unshowViewable(Viewable *who) override;
    void entityTurned(Entity *who, char direction) override;
    void showDoor(Door *door) override;
    void showDoors(const std::vector<Door *> &doors) override;
    void sendMessage(const char *text, unsigned char channel = 3) override;
    void entityMoved(Entity *who, char direction) override;
    const std::vector<Tracker *> &getTrackers()
    {
        return trackers;
    }

    static log4cplus::Logger log();

private:

    Equipment *removeEquipment(int slot);
    void eqChangeMessage(Equipment *eq, Equipment::Slots slot);

    Stats baseStats;
    int curMp, nDeaths;
    long long exp, ap;
    unsigned char hairstyle, haircolor, statPoints;
    char gender;
    unsigned char level, ab;
    std::vector<LegendItem> legend;
    Path path;
    int pathMask;
    int regenCount, mpRegenCount;
    int charId, settings;
    unsigned short maxWeight, eqpWgt, priv, itemLimit, walkLimit, secretLimit,
        limitTicks, laborCount, nation;
    unsigned int gold, storedGold, laborReset;
    unsigned char chantCount, chantSet, chantFree;bool ghost, refreshed,
        updateDb;
    Group *group;

    CharacterSession *session;
    Guild *guild;
    Guild::Rank rank;
    Field *f;
    TradeView *trade;

    std::vector<StorageItem> storage;

    Skill *skills[NUM_SKILLS];
    Secret *secrets[NUM_SKILLS];
    Inventory<MAX_EXCHANGE> exchange;
    Inventory<NUM_ITEMS> inventory;
    std::vector<Item *> lostItems;
    std::vector<Tracker *> trackers;

    Equipment *equipment[NUM_EQUIPS];
    std::map<int, QuestProgress> quests;

};

#endif /* CHARACTER_H_ */
