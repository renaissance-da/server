/*
 * Character.cpp
 *
 *  Created on: 2011-09-07
 *      Author: per
 */

#include "Character.h"
#include "Skill.h"
#include <string.h>
#include "random_engines.h"
#include "GameTime.h"
#include <stdio.h>
#include "DataService.h"
#include "npc_bindings.h"
#include "DataLoaders.h"
#include "Equipment.h"
#include "Consumable.h"
#include "NPC.h"
#include "Combat.h"
#include <time.h>
#include "log4cplus/loggingmacros.h"
#include "config.h"
#include "Mob.h"

#include "defines.h"
#include <assert.h>
#include <algorithm>

#ifdef WIN32
#define snprintf _snprintf
#endif

log4cplus::Logger Character::log()
{
    return log4cplus::Logger::getInstance("renaissance.character");
}

char getShowSlot(int slot)
{
    int r;
    if (slot < 35)
        r = slot + 1;
    else if (slot < 70)
        r = slot + 2;
    else
        r = slot + 3;

    return r;
}

int getActSlot(int slot)
{
    int actSlot;
    if (slot < 1)
        return -1;
    else if (slot < 36)
        actSlot = slot - 1;
    else if (slot < 72)
        actSlot = slot - 2;
    else if (slot < 89)
        actSlot = slot - 3;
    else
        return -1;

    return actSlot;
}

Character::Character(int id, const char *name, Map *map, unsigned short x,
    unsigned short y, int hp, int mp, int maxHp, int maxMp, short strength,
    short int_, short wis, short dex, short con, unsigned char hairstyle,
    unsigned char haircolor, unsigned char statPoints, char gender, short path,
    int pathmask, short level, short ab, long long exp, long long ap,
    unsigned int gold, unsigned int storedGold, unsigned short priv,
    bool dead, unsigned short labor, unsigned int laborReset,
    unsigned short nation, int settings) :
    Entity(x, y, map, name), curMp(mp), nDeaths(0), exp(exp), ap(ap), hairstyle(
        hairstyle), haircolor(haircolor), statPoints(statPoints), gender(
        gender), level((unsigned char) level), ab((unsigned char) ab), path(
        (Path) path), pathMask(pathmask), regenCount(0), mpRegenCount(0), charId(
        id), settings(settings), eqpWgt(0), priv(priv), itemLimit(0), walkLimit(
        0), secretLimit(0), limitTicks(0), laborCount(labor), nation(nation), gold(
        gold), storedGold(storedGold), laborReset(laborReset), chantCount(0), chantSet(
        0), chantFree(0), ghost(dead), refreshed(false), updateDb(true), group(
        0), session(0), guild(0), rank(Guild::MEMBER), f(0), trade(0)
{
    this->hp = hp;
    baseStats.setStr(strength);
    baseStats.setInt(int_);
    baseStats.setWis(wis);
    baseStats.setDex(dex);
    baseStats.setCon(con);
    baseStats.setMaxHp(maxHp);
    baseStats.setMaxMp(maxMp);
    baseStats.setAc(100 - level / 3);

    stats = baseStats;

    maxWeight = 48 + baseStats.getStr() + level / 4;

    for (int i = 0; i < NUM_SKILLS; i++) {
        skills[i] = 0;
        secrets[i] = 0;
    }

    for (int i = 0; i < NUM_EQUIPS; i++) {
        equipment[i] = 0;
    }

    //Add the generic tracker
    trackers.push_back(new Tracker());
}

void Character::setNationCoords()
{
    switch (nation) {
    Map *m;
    case 1: //mileth
        m = DataService::getService()->getMap(136);
        assert(m);
        setMap(m);
        setX(6);
        setY(8);
        break;
    default:
        m = DataService::getService()->getMap(3006);
        assert(m);
        setMap(m);
        setX(8);
        setY(8);
        break;
    }
}

void Character::addGold(int amt)
{
    gold = (int) gold + amt > 0 ? gold + amt : 0;
    Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
}

bool Character::useLabor(unsigned short amt)
{
    //check reset
    time_t tnow;
    time(&tnow);
    if (tnow - laborReset > LABOR_RESET_INTERVAL) {
        laborReset = tnow;
        laborCount = MAX_LABOR;
    }
    if (amt > laborCount)
        return false;
    laborCount -= amt;
    return true;
}

const char *Character::getLegendTextParam(int legendId)
{
    for (LegendItem &li : legend) {
        if (li.base->getId() == legendId)
            return li.textParam;
    }
    return 0;
}

int Character::getLegendTimestamp(int legendId)
{
    for (LegendItem &li : legend) {
        if (li.base->getId() == legendId)
            return li.timestamp;
    }
    return 0;
}

void Character::getLaborCountTime(unsigned short &count, unsigned int &ltime)
{
    //check reset
    time_t tnow;
    time(&tnow);
    if (tnow - laborReset > LABOR_RESET_INTERVAL) {
        laborReset = tnow;
        laborCount = MAX_LABOR;
    }
    count = laborCount;
    ltime = laborReset;
}

const char *Character::getGender()
{
    if (gender == 'm')
        return "m";
    else
        return "f";
}

void Character::cancelTrade()
{
    if (!trade)
        return;
    trade->cancel();
    Character *o = trade->getOther();
    delete o->trade;
    o->trade = 0;
    trade->deleteExchange();
    delete trade;
    trade = 0;
}
void Character::confirmTrade()
{
    if (!trade)
        return;
    if (trade->confirm()) {
        Character *o = trade->getOther();
        delete o->trade;
        o->trade = 0;
        trade->deleteExchange();
        delete trade;
        trade = 0;
    }
}
void Character::giveItem(int oid, char slot)
{
    if (!canAct())
        return;
    //Tries to give an item to an entity
    int actSlot = slot - 1;
    Map *m = getMap();
    Entity *e = m->getNearByOid(this, oid, 4);

    //Exchange distances work a little oddly. Instead of the 1-norm being < 3, the
    //infinity norm must be < 3
    if (!e || DIFF(e->getX(), getX()) > 2 || DIFF(e->getY(), getY()) > 2)
        return;
    switch (e->getType()) {
    case Entity::E_CHARACTER:
        //ask to start a trade - gold wont trigger giveItem but exchanges can be started by giving gold too
        Server::initExchange(session, oid, slot);
        break;
    case Entity::E_MOB:
        if (inventory[actSlot]) {
            if (inventory[actSlot]->isBound())
                Server::sendMessage(session, "You can't exchange this item.");
            else
                ((Mob *) e)->giveItem(inventory.remove(actSlot, this));
        }
        break;
    default:
        //TODO implement for npcs
        break;
    }
}

/**
 * \brief Makes this character attempt to give gold to another entity
 *
 * Makes this character attempt to give gold to another entity. To do so,
 * the character must have enough gold, and the target entity must be near.
 * \param[in] oid The identifier of the entity to give gold to
 * \param[in] amt The amount of gold to give to the target entity
 */
void Character::giveGold(int oid, unsigned int amt)
{
    if (!canAct())
        return;

    Map *m = getMap();
    Entity *e = m->getNearByOid(this, oid, 4);
    if (!e || DIFF(e->getX(), getX()) > 2 || DIFF(e->getY(), getY()) > 2)
        return;

    switch (e->getType()) {
    case Entity::E_CHARACTER:
        //Start an exchange
        Server::initGoldExchange(session, oid, amt);
        break;
    default:
        //Unlike items, cannot give gold to non-players
        break;
    }
}

void Character::startTrade(int oid)
{
    Map *m = getMap();
    Entity *e = m->getNearByOid(this, oid);
    if (e->getType() != E_CHARACTER) {
        //TODO bad trader
        return;
    }
    Character *r = (Character *) e;
    if (!r->canAct()) {
        Server::sendMessage(session, "They are busy.");
        return;
    }
    else if (!(r->getSettings() & EXCHANGE)) {
        Server::sendMessage(session, "They refuse to exchange.");
        return;
    }
    Exchange *ex = new Exchange(this, r);
    trade = new TradeView(ex, 0);
    r->trade = new TradeView(ex, 1);
    Server::startExchangeWith(session, oid, r->getName().c_str());
    Server::startExchangeWith(r->session, getOid(), getName().c_str());
}
void Character::tradeItem(int oid, char slot)
{
    int actSlot = slot - 1;

    //TODO could check oid here
    if (!trade)
        return;
    if (actSlot < 0 || actSlot >= NUM_ITEMS || !inventory[actSlot])
        return;

    if (inventory[actSlot]->isBound()) {
        Server::sendMessage(session, "You can't trade this item.");
        return;
    }

    if (inventory[actSlot]->getMaxQty())
        Server::exchangeHowMany(session, slot);
    else if (trade->addItem(inventory[actSlot]))
        inventory.remove(actSlot, this);
    //removeItem(actSlot);
}
void Character::tradeItem(int oid, char slot, char amt)
{
    int actSlot = slot - 1;
    int iamt = (int) amt;
    //TODO could check oid here
    if (!trade)
        return;
    if (actSlot < 0 || actSlot >= NUM_ITEMS || !inventory[actSlot])
        return;

    if (inventory[actSlot]->isBound())
        Server::sendMessage(session, "You can't trade this item.");
    else if (inventory[actSlot]->getQty() < iamt)
        Server::sendMessage(session, "You don't have that many.");
    else {
        Item *split = inventory.removeSome(actSlot, iamt, this);
        if (!trade->addItem(split))
            if (!inventory.addItem(split, this))
                lostItems.push_back(split);
    }
}

void Character::tradeGold(int oid, int amt)
{
    if (!trade)
        return;
    if (amt < 0 || 0 > ((int) gold - amt)) {
        Server::sendMessage(session, "You don't have enough gold.");
        return;
    }
    if (trade->addGold(amt))
        addGold(-amt);
}

/**
 * \brief Test if the character can take actions
 *
 * Test if the character can take actions. It's possible that a character can
 * move but not take actions
 * \return True if the character is allowed to take actions (activate abilities, trade)
 */
bool Character::canAct()
{
    return !trade && !ghost && Entity::canAct();
}

unsigned char Character::getAtkAnim()
{
    if (equipment[Equipment::WEAPON]) {
        //weapon determines animation
        if (equipment[Equipment::WEAPON]->getFlags() & 1) {
            //Monk weapon
            return 0x84;
        }
        else if (equipment[Equipment::WEAPON]->getFlags() & 4) {
            //Two handed weapon
            if (skillLevel(16) >= 0)
                return 0x81;
            else
                return 1;
        }
        //TODO check for bow
        else {
            return 1;
        }
    }
    else {
        if (path == Monk)
            return 0x84;
        return 1;
    }
}

void Character::cancelCasting()
{
    chantSet = chantCount = 0;
}

bool Character::useSkill(char slot)
{
    if (!canAct())
        return false;
    cancelCasting();

    int actSlot;
    if (slot < 36)
        actSlot = slot - 1;
    else if (slot < 72)
        actSlot = slot - 2;
    else if (slot < 90)
        actSlot = slot - 3;
    else
        return false;

    if (skills[actSlot] && !skills[actSlot]->onCd()) {

        //check restrictions
        if ((skills[actSlot]->getFlags() & MONK_SKILL)
            && (equipment[Equipment::SHIELD]
                || (equipment[Equipment::WEAPON]
                    && !(equipment[Equipment::WEAPON]->getFlags() & 1)))) {
            Server::sendMessage(session, "You can't do it.");
            skills[actSlot]->used(false);
            Server::startCd(session, 1, slot, skills[actSlot]->getBase()->cd);
            return false;
        }
        if ((skills[actSlot]->getFlags() & ROGUE_SKILL)
            && !(equipment[Equipment::WEAPON]
                && (equipment[Equipment::WEAPON]->getFlags() & 2))) {
            Server::sendMessage(session, "You can't do it.");
            skills[actSlot]->used(false);
            Server::startCd(session, 1, slot, skills[actSlot]->getBase()->cd);
            return false;
        }

        bool imp = Combat::useSkill(this, skills[actSlot]);
        skills[actSlot]->used(imp);

        //Did the skill level?
        if (skills[actSlot]->canLevel()) {
            skills[actSlot]->levelUp();
            char buffer[100];
            snprintf(buffer, 100, "%s has improved.",
                skills[actSlot]->getName().c_str());
            Server::sendMessage(session, buffer);
            Server::getSkill(session, skills[actSlot], slot);
        }

        Server::startCd(getSession(), 1, slot, skills[actSlot]->getBase()->cd);

        return true;
    }
    return false;
}

void Character::setHairstyle(unsigned char style)
{
    hairstyle = style;
    appearanceChanged();

    if (updateDb)
        DataService::getService()->updateHairstyle(charId, style);
}

void Character::setHaircolor(unsigned char color)
{
    haircolor = color;
    appearanceChanged();

    if (updateDb)
        DataService::getService()->updateHaircolor(charId, color);
}

void Character::setNation(unsigned short n)
{
    nation = n;
    if (updateDb)
        DataService::getService()->updateNation(charId, n);
}

Element Character::getAtkEle()
{
    if (equipment[Equipment::NECKLACE])
        return equipment[Equipment::NECKLACE]->getElement();
    return None;
}

Element Character::getDefEle()
{
    if (equipment[Equipment::BELT])
        return equipment[Equipment::BELT]->getElement();
    return None;
}

bool Character::useSecret(char slot, int oid)
{
    Map *m = getMap();

    if (m->noSecrets()) {
        Server::sendMessage(session, "That doesn't work here.");
        return false;
    }

    int actSlot;
    if (slot < 36)
        actSlot = slot - 1;
    else if (slot < 72)
        actSlot = slot - 2;
    else if (slot < 90)
        actSlot = slot - 3;
    else
        return false;

    if (secrets[actSlot] && !secrets[actSlot]->onCd()) {
        //Check if the player is casting too fast. This is separate of the instant cast test
        int weapon =
            equipment[Equipment::WEAPON] ?
                equipment[Equipment::WEAPON]->getId() : 0;
        int castTime = TICKS * secrets[actSlot]->getCastTime((Staff) weapon);
        if (castTime == 0
            || (chantSet == castTime && chantCount + chantFree >= chantSet)) {
            chantFree = chantSet == chantCount ? 0 : 1;
            chantSet = 0;
            chantCount = 0;
        }
        else {
            //Casted too soon
            return false;
        }

        //Check for silence/frozen

        //Check casting limit
        if (secretLimit > SECRET_LIMIT) {
            return false;
        }
        secretLimit++;

        //Find target
        Entity *t = 0;
        if (secrets[actSlot]->isTargeted()) {
            t = m->getNearByOid(this, oid);
            if (!t)
                return false;
        }
        char buffer[100];

        bool imp = Combat::useSkill(this, secrets[actSlot], t);
        secrets[actSlot]->used(imp, false);

        if (secrets[actSlot]->canLevel()) {
            secrets[actSlot]->levelUp();
            snprintf(buffer, 100, "%s has improved.",
                secrets[actSlot]->getName().c_str());
            Server::sendMessage(session, buffer);
            int weapon = (
                equipment[Equipment::WEAPON] ?
                    equipment[Equipment::WEAPON]->getId() : 0);
            Server::getSecret(session, secrets[actSlot], slot, weapon);
        }

        Server::startCd(session, 0, slot, secrets[actSlot]->getBase()->cd);

        return true;
    }
    return false;
}

/**
 * Tries to learn the skill given.
 * \param si The skill to learn
 * \return The slot which the skill is placed in, between 1 and 87, or 0 if no slots were left
 */
char Character::learnSkill(Skill *sk)
{
    unsigned short minslot, maxslot;
    char actSlot;

    if (sk->getFlags() & UTILITY) {
        minslot = 70;
        maxslot = 87;
        actSlot = 3;
    }
    else {
        minslot = 0;
        maxslot = 35;
        actSlot = 1;
    }

    for (unsigned short i = minslot; i < maxslot; i++) {
        if (!skills[i]) {
            skills[i] = sk;
            Server::getSkill(session, sk, i + actSlot);
            if (updateDb)
                DataService::getService()->addSkill(charId, sk->getId(), i);
            return i + actSlot;
        }
    }
    return 0;
}

StatusEffect *Character::addStatusEffect(int skid)
{
    StatusEffect *se = Entity::addStatusEffect(skid);

    if (!se)
        return 0;
    Stats stchg;
    se->getChange(stchg);
    stats += stchg;

    Server::sendStatusEffect(session, se->getIcon(), se->getDur());
    Server::sendMessage(session, se->getReceivedMsg());
    Server::updateStatInfo(session, this, FLAG_ALL);
    return se;
}

StatusEffect *Character::addStatusEffect(int skid, int dur)
{
    StatusEffect *se = Entity::addStatusEffect(skid, dur);
    if (!se)
        return 0;
    Stats stchg;
    se->getChange(stchg);
    stats += stchg;

    Server::sendStatusEffect(session, se->getIcon(), se->getDur());
    Server::sendMessage(session, se->getReceivedMsg());
    Server::updateStatInfo(session, this, FLAG_ALL);
    return se;
}

bool Character::removeEffect(StatusEffect::Kind kind)
{
    if (!effects[kind])
        return false;

    const char *message = effects[kind]->getEndedMsg();
    unsigned short icon = effects[kind]->getIcon();

    if (kind == StatusEffect::DOOM) {
        if (effects[kind]->getTickDuration() == 0) {
            message = NULL;
        }
        else
            hp = 1;
    }

    bool res = Entity::removeEffect(kind);
    if (res) {
        Server::sendStatusEffect(session, icon, 0);
        if (message)
            Server::sendMessage(session, message);
        Server::updateStatInfo(session, this, FLAG_ALL);
    }

    return res;
}

bool Character::removeSkill(int skid)
{
    for (int i = 0; i < NUM_SKILLS; i++) {

        if (skills[i] && skills[i]->getId() == skid) {
            Server::eraseSkill(session, getShowSlot(i));
            delete skills[i];
            skills[i] = 0;
            if (updateDb)
                DataService::getService()->removeSkill(charId, i);
            return true;
        }
        if (secrets[i] && secrets[i]->getId() == skid) {
            Server::eraseSecret(session, getShowSlot(i));
            delete secrets[i];
            secrets[i] = 0;
            if (updateDb)
                DataService::getService()->removeSecret(charId, i);
            return true;
        }
    }
    return false;
}

void Character::forgetSecret(int slot)
{
    int actSlot = getActSlot(slot);
    if (actSlot < 0)
        return;

    if (!secrets[actSlot])
        return;

    delete secrets[actSlot];
    secrets[actSlot] = 0;
    Server::eraseSecret(session, slot);
    if (updateDb)
        DataService::getService()->removeSecret(charId, actSlot);
}

void Character::forgetSkill(int slot)
{
    int actSlot = getActSlot(slot);
    if (actSlot < 0)
        return;

    if (!skills[actSlot])
        return;

    delete skills[actSlot];
    skills[actSlot] = 0;
    Server::eraseSkill(session, slot);
    if (updateDb)
        DataService::getService()->removeSkill(charId, actSlot);
}

int Character::getQuestProgress(int questId)
{
    auto q = quests.find(questId);
    if (q == quests.end())
        return 0;
    return q->second.progress;
}
void Character::setQuestProgress(int questId, int progress)
{
    if (!quests.count(questId)) {
        quests[questId].progress = progress;
        quests[questId].timer = 0;
        if (updateDb)
            DataService::getService()->addQuest(charId, questId, progress, 0);
    }
    else {
        quests[questId].progress = progress;
        if (updateDb)
            DataService::getService()->updateQuest(charId, questId, progress,
                quests[questId].timer);
    }
}
int Character::getQuestTimer(int questId)
{
    auto q = quests.find(questId);
    if (q == quests.end())
        return 0;
    return q->second.timer;
}
void Character::setQuestTimer(int questId, int timer)
{
    if (!quests.count(questId)) {
        QuestProgress qp;
        qp.progress = 0;
        qp.timer = timer;
        quests[questId] = qp;
        if (updateDb)
            DataService::getService()->addQuest(charId, questId, 0, timer);
    }
    else {
        quests[questId].timer = timer;
        if (updateDb)
            DataService::getService()->updateQuest(charId, questId,
                quests[questId].progress, timer);
    }
}

char Character::learnSecret(Secret *sp)
{
    int minslot, maxslot;
    char actSlot;

    if (sp->getFlags() & UTILITY) {
        minslot = 70;
        maxslot = 87;
        actSlot = 3;
    }
    else {
        minslot = 0;
        maxslot = 35;
        actSlot = 1;
    }

    for (int i = minslot; i < maxslot; i++) {
        if (!secrets[i]) {
            secrets[i] = sp;
            int weapon =
                equipment[Equipment::WEAPON] ?
                    equipment[Equipment::WEAPON]->getId() : 0;
            Server::getSecret(session, sp, i + actSlot, weapon);
            if (updateDb)
                DataService::getService()->addSecret(charId, sp->getId(), i);
            return i + actSlot;
        }
    }
    return 0;
}

void Character::attack()
{
    for (int i = 0; i < 35; i++) {
        if (skills[i] && skills[i]->isAssail() && !skills[i]->onCd()) {
            useSkill(i + 1);
        }
    }
}

void Character::clickGround(unsigned short x, unsigned short y)
{
    Map *m = getMap();

    //Check for doors
    Door *d = m->getDoor(x, y);
    if (d) {
        d->toggle();
        Server::sendMessage(session, d->getDesc());
    }
    //Check for signs

}

void Character::getProfileInfo(IDataStream *dp)
{
    dp->appendInt(getOid());

    //shown equipment
    for (int i = 1; i < NUM_EQUIPS; i++) {
        dp->appendShort(equipment[i] ? equipment[i]->getApr() : 0);
        dp->appendByte(0);
    }

    dp->appendByte(0);
    dp->appendString(name.length(), name.c_str());
    dp->appendByte(4); //Town emblem!
    dp->appendString(0, ""); //Title!
    dp->appendByte((settings & JOIN_A_GROUP) / JOIN_A_GROUP); // 1 = group 0 = alone, DO NOT change the high bits from 0, the client will die
    const char *rankName = 0;
    if (guild) {
        rankName = guild->getRankName(rank);
        dp->appendString(strlen(rankName), rankName);
    }
    else
        dp->appendString(0, "");
    if (!isMaster())
        dp->appendString(paths[path].nameLen, paths[path].name);
    else
        dp->appendString(paths[Master].nameLen, paths[Master].name);

    // Guild name!
    if (guild)
        dp->appendString(guild->getName().length(), guild->getName().c_str());
    else
        dp->appendString(0, "");

    dp->appendByte(legend.size());
    for (auto it = legend.begin(); it != legend.end(); it++) {
        dp->appendByte(it->base->getIcon());
        dp->appendByte(it->base->getColor());
        dp->appendString(strlen(it->base->getPrefix()), it->base->getPrefix());
        dp->appendString(it->textlen, it->text);
    }

    //some blanks
    dp->appendShort(0);
    dp->appendByte(0);
}

void Character::clickEntity(int oid)
{
    Map *m = getMap();
    Entity *e = m->getNearByOid(this, oid);
    if (e) {
        switch (e->getType()) {
        case Entity::E_MOB:
            Server::sendMessage(session, e->getName().c_str());
            break;
        case Entity::E_NPC:
            initDlg(this, (NPC *) e);
            break;
        case Entity::E_CHARACTER:
            Server::profileInfo(session, (Character *) e);
            break;
        default:
            break;
        }
    }
}

const Stats *Character::getStats()
{
    return &stats;
}

void Character::damage(Entity *a, int amt)
{
    if (effects[StatusEffect::DOOM] || ghost)
        return;

    Entity::damage(a, amt);
    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
}

void Character::heal(float amt, bool show)
{
    if (ghost || effects[StatusEffect::DOOM])
        return;

    Entity::heal(amt, show);
    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
}

void Character::weaponStruck()
{
    if (equipment[Equipment::WEAPON]) {
        equipment[Equipment::WEAPON]->decDur();
        if (equipment[Equipment::WEAPON]->getDur() <= 0) {
            //broken!
            stats -= *(equipment[Equipment::WEAPON]->getStats());
            eqpWgt -= equipment[Equipment::WEAPON]->getWeight();
            Server::sendMessage(session, "Broken.");
            Server::removeEquip(session, Equipment::WEAPON);
            delete equipment[Equipment::WEAPON];
            equipment[Equipment::WEAPON] = 0;
            appearanceChanged();
            Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);
        }
    }
}

void Character::showAction(Viewable *who, char action, short duration)
{
    if (canSee(who))
        Server::entActed(session, who->getOid(), action, duration);
}

void Character::entityStruck(Entity *who, unsigned short dispHp,
    unsigned char sound)
{
    if (canSee(who))
        Server::entStruck(session, who->getOid(), dispHp, sound);
}

void Character::playSound(unsigned char sound)
{
    Server::playSound(session, sound);
}

void Character::unshowViewable(Viewable *who)
{
    Server::entGone(session, who->getOid());
}

void Character::entityTurned(Entity *who, char direction)
{
    if (canSee(who))
        Server::entTurned(session, who);
}

void Character::showDoor(Door *door)
{
    Server::showDoor(session, door);
}

void Character::showDoors(const std::vector<Door *> &doors)
{
    Server::sendDoors(session, doors);
}

void Character::sendMessage(const char *text, unsigned char channel)
{
    Server::sendMessage(session, text, channel);
}

void Character::entityMoved(Entity *who, char direction)
{
    if (this == who)
        Server::movedSelf(session, getX(), getY(), direction);
    else if (canSee(who))
        Server::entMoved(session, who, direction);
}

void Character::showEffect(Viewable *who, unsigned short effectId, int duration)
{
    if (canSee(who))
        Server::playEffect(session, who->getOid(), effectId, duration);
}

void Character::swapSecrets(char dst, char src)
{
    int trueDst, trueSrc;
    if (dst < 36 && src < 36) {
        trueDst = dst - 1;
        trueSrc = src - 1;
    }
    else if (dst > 36 && dst < 72 && src > 36 && src < 72) {
        trueDst = dst - 2;
        trueSrc = src - 2;
    }
    else if (dst > 72 && dst < 90 && src > 72 && src < 90) {
        trueDst = dst - 3;
        trueSrc = src - 3;
    }
    else
        return;

    if (trueDst == trueSrc)
        return;

    int weapon =
        equipment[Equipment::WEAPON] ?
            equipment[Equipment::WEAPON]->getId() : 0;

    //Swapping
    if (secrets[trueDst] && secrets[trueSrc]) {
        Secret *tmp = secrets[trueSrc];
        Server::getSecret(getSession(), secrets[trueDst], src, weapon);
        Server::getSecret(getSession(), secrets[trueSrc], dst, weapon);
        secrets[trueSrc] = secrets[trueDst];
        secrets[trueDst] = tmp;
        if (updateDb)
            DataService::getService()->swapSecrets(charId, trueSrc, trueDst,
                secrets[trueDst]->getId(), secrets[trueSrc]->getId());
    }
    //Moving in reverse, client usually can't issue this command but we support it
    else if (secrets[trueDst]) {
        Server::eraseSecret(getSession(), dst);
        Server::getSecret(getSession(), secrets[trueDst], src, weapon);
        secrets[trueSrc] = secrets[trueDst];
        secrets[trueDst] = 0;
        if (updateDb)
            DataService::getService()->moveSecret(charId, trueDst, trueSrc);
    }
    //Normal direction moving
    else if (secrets[trueSrc]) {
        Server::eraseSecret(getSession(), src);
        Server::getSecret(getSession(), secrets[trueSrc], dst, weapon);
        secrets[trueDst] = secrets[trueSrc];
        secrets[trueSrc] = 0;
        if (updateDb)
            DataService::getService()->moveSecret(charId, trueSrc, trueDst);
    }
}

void Character::swapSkills(char dst, char src)
{
    int trueDst, trueSrc;
    if (dst < 36 && src < 36) {
        trueDst = dst - 1;
        trueSrc = src - 1;
    }
    else if (dst > 36 && dst < 72 && src > 36 && src < 72) {
        trueDst = dst - 2;
        trueSrc = src - 2;
    }
    else if (dst > 72 && dst < 90 && src > 72 && src < 90) {
        trueDst = dst - 3;
        trueSrc = src - 3;
    }
    else
        return;

    if (trueDst == trueSrc)
        return;

    //Swapping
    if (skills[trueDst] && skills[trueSrc]) {
        Skill *tmp = skills[trueSrc];
        Server::getSkill(getSession(), skills[trueDst], src);
        Server::getSkill(getSession(), skills[trueSrc], dst);
        skills[trueSrc] = skills[trueDst];
        skills[trueDst] = tmp;
        if (updateDb)
            DataService::getService()->swapSkills(charId, trueSrc, trueDst,
                skills[trueDst]->getId(), skills[trueSrc]->getId());
    }
    //Moving in reverse, client usually can't issue this command but we support it
    else if (skills[trueDst]) {
        Server::eraseSkill(getSession(), dst);
        Server::getSkill(getSession(), skills[trueDst], src);
        skills[trueSrc] = skills[trueDst];
        skills[trueDst] = 0;
        if (updateDb)
            DataService::getService()->moveSkill(charId, trueDst, trueSrc);
    }
    //Normal direction moving
    else if (skills[trueSrc]) {
        Server::eraseSkill(getSession(), src);
        Server::getSkill(getSession(), skills[trueSrc], dst);
        skills[trueDst] = skills[trueSrc];
        skills[trueSrc] = 0;
        if (updateDb)
            DataService::getService()->moveSkill(charId, trueSrc, trueDst);
    }

}

void Character::swapInv(char dst, char src)
{
    if (!canAct())
        return;
    int actSrc = src - 1;
    int actDst = dst - 1;
    if (actSrc < 0 || actSrc >= NUM_ITEMS || actDst < 0 || actDst >= NUM_ITEMS)
        return;

    inventory.swap(actSrc, actDst, this);
}

Character::~Character()
{
    for (int i = 0; i < NUM_SKILLS; i++) {
        if (skills[i])
            delete skills[i];
        if (secrets[i])
            delete secrets[i];
    }
    for (Tracker *t : trackers) {
        delete t;
    }
}

void Character::startChant(unsigned char count)
{
    chantSet = count * TICKS;
    chantCount = 0;
}

void Character::getLegendInfo(IDataStream *dp)
{
    dp->appendByte(4); //4=mileth, 7=noes
    const char *rankName = "";
    if (guild) {
        rankName = guild->getRankName(rank);
    }
    dp->appendString(strlen(rankName), rankName);
    dp->appendString(0, ""); // title

    //TODO there are some issues with group synchronization which arise because the group is
    //protected by the dataservice lock
    Map *m = getMap();
    DataService::getService()->groupListMembers(dp, this);

    dp->appendByte((settings & JOIN_A_GROUP) / JOIN_A_GROUP);
    dp->appendByte(0); //has to be a byte, not sure what it does
    dp->appendByte(path);
    //dp->appendShort(0); // was 1, 1 on archer, 0,1 on master priest
    dp->appendByte(0); // is med path, probably
    dp->appendByte(isMaster() ? 1 : 0); // is master, probably
    if (!isMaster())
        dp->appendString(paths[path].nameLen, paths[path].name);
    else
        dp->appendString(paths[Master].nameLen, paths[Master].name);
    if (guild)
        dp->appendString(guild->getName().length(), guild->getName().c_str());
    else
        dp->appendString(0, "");
    dp->appendByte(legend.size());
    for (unsigned short i = 0; i < legend.size(); i++) {
        dp->appendByte(legend[i].base->getIcon());
        dp->appendByte(legend[i].base->getColor());
        dp->appendString(strlen(legend[i].base->getPrefix()),
            legend[i].base->getPrefix());
        dp->appendString(legend[i].textlen, legend[i].text);
    }
}

/**
 * \brief Write out a (mostly) readable description of the character's
 * server-side settings.
 *
 * Writes out the character's server side settings. These are (in order)
 * listen to whisper, join a group, listen to shout, believe in wisdom,
 * believe in magic, exchange, "fast move" (show group window), and
 * clan whisper. Each setting is sent as a fixed length string of 21 chars
 * plus a 1 byte prefix which doesn't seem to mean anything.
 * \param[in] dp A Data stream pointer to which the result will be written.
 * \param[in,opt] which Which setting to get. If 0, gets all settings.
 */
void Character::getSettings(IDataStream *dp, int which)
{
    dp->appendByte(0);
    if (!which) {
        const int offset = 22;

        char data[0xb1] = "0Listen to whisper:OFF"
            "\tJoin a group     :OFF"
            "\tListen to shout  :OFF"
            "\tBelieve in wisdom:OFF"
            "\tBelieve in magic :OFF"
            "\tExchange         :OFF"
            "\tFast Move        :OFF"
            "\tClan whisper     :OFF";

        if (settings & LISTEN_TO_WHISPERS) {
            data[20] = 'N';
            data[21] = ' ';
        }
        if (settings & JOIN_A_GROUP) {
            data[20 + offset * 1] = 'N';
            data[21 + offset * 1] = ' ';
        }
        if (settings & LISTEN_TO_SHOUT) {
            data[20 + offset * 2] = 'N';
            data[21 + offset * 2] = ' ';
        }
        if (settings & BELIEVE_IN_WISDOM) {
            data[20 + offset * 3] = 'N';
            data[21 + offset * 3] = ' ';
        }
        if (settings & BELIEVE_IN_MAGIC) {
            data[20 + offset * 4] = 'N';
            data[21 + offset * 4] = ' ';
        }
        if (settings & EXCHANGE) {
            data[20 + offset * 5] = 'N';
            data[21 + offset * 5] = ' ';
        }
        if (settings & GROUP_WINDOW) {
            data[20 + offset * 6] = 'N';
            data[21 + offset * 6] = ' ';
        }
        if (settings & CLAN_WHISPER) {
            data[20 + offset * 7] = 'N';
            data[21 + offset * 7] = ' ';
        }
        dp->appendString(0xB0, data);
    }

    else if (which > 0 && which < 9) {
        char data[23];
        const char *field;
        switch (which) {
        case 1:
            field = "Listen to whisper";
            break;
        case 2:
            field = "Join a group";
            break;
        case 3:
            field = "Listen to shout";
            break;
        case 4:
            field = "Believe in wisdom";
            break;
        case 5:
            field = "Believe in magic";
            break;
        case 6:
            field = "Exchange";
            break;
        case 7:
            field = "Fast Move";
            break;
        case 8:
            field = "Clan whisper";
            break;
        }

        snprintf(data, 23, "%d%-17s:%s", which, field,
            (settings & (1 << (which - 1))) ? "ON " : "OFF");
        dp->appendString(22, data);
    }

}

/**
 * Toggle one of the character's server-side settings
 *
 * \param[in] which Which setting to toggle. If between 1 and 8, toggles the
 * corresponding setting as given in getSettings. Otherwise does nothing.
 */
void Character::toggleSetting(int which)
{
    switch (which) {
    case 1:
        settings ^= LISTEN_TO_WHISPERS;
        break;
    case 2:
        settings ^= JOIN_A_GROUP;
        if (group)
            DataService::getService()->leaveGroup(this);
        break;
    case 3:
        settings ^= LISTEN_TO_SHOUT;
        break;
    case 4:
        settings ^= BELIEVE_IN_WISDOM;
        break;
    case 5:
        settings ^= BELIEVE_IN_MAGIC;
        break;
    case 6:
        settings ^= EXCHANGE;
        break;
    case 7:
        settings ^= GROUP_WINDOW;
        break;
    case 8:
        settings ^= CLAN_WHISPER;
        break;
    default:
        //Do nothing
        break;
    }
}

void Character::finishExchange(Inventory<MAX_EXCHANGE> *exchange)
{
    Item **eitems = exchange->allItems();

    for (int i = 0; i < MAX_EXCHANGE; i++) {
        if (eitems[i]) {
            if (!getItem(eitems[i]))
                lostItems.push_back(eitems[i]);
            *(eitems + i) = 0;
        }
    }
}

unsigned int Character::getExchangeGold()
{
    if (trade)
        return trade->getOfferedGold();
    else
        return 0;
}

/**
 * \brief Changes the character's class
 *
 * Changes the character's class to the class enumerated by the value given. If p does not enumerate a path,
 * then this method does nothing.
 * \param p The value enumerating the path to be given
 */
void Character::setPath(unsigned short p)
{
    if (p < Path_Max) {
        path = (Path) p;
        pathMask |= paths[p].mask;
        if (updateDb)
            DataService::getService()->updatePath(charId, path, pathMask);
    }
}

/**
 * Gets the level of the given skill on this player
 *
 * Gets the level of the given skill on this player, or -1. Also looks through secrets.
 * \param id The skill id to look for
 * \return The level of the skill, or -1 if the skill is not learned
 */
int Character::skillLevel(int id)
{
    for (int i = 0; i < NUM_SKILLS; i++) {
        if (skills[i] && skills[i]->getId() == id)
            return skills[i]->getLevel();
    }
    for (int i = 0; i < NUM_SKILLS; i++) {
        if (secrets[i] && secrets[i]->getId() == id)
            return secrets[i]->getLevel();
    }
    return -1;
}

void Character::getViewedBlock(IDataStream *op)
{
    op->appendShort(getX());
    op->appendShort(getY());
    op->appendByte(getDir());
    op->appendInt(getOid());
    //op->appendInt(0x000F2000); //0D15 gave bright blue pants (00 hairstyle (color of pants | boyflag) 00), think boys will be 0x01xx, girls 0x02xx
    /*op->appendByte(0); //think haircolor goes here, was 0
     op->appendByte(hairstyle);
     op->appendByte((gender == 'm' ? 0x11 : 0x21)); //this field works in weird ways. 1 causes you to have no face, but 10001 is male, with face. 100000 is female
     op->appendByte(0); //doesn't change anything apparently*/
    //op->appendInt(appearanceData); //ok, so first short is hair/hat graphic, next byte has a lo nybble specifying a highlight, hi nybble is type
    //some types: 10 = boy, 20 = girl, 30 = boy ghost, 40 = girl ghost, 50 = hidden boy, 60 = hidden girl, weird ones follow.
    //4th byte probably ignored
    if (equipment[Equipment::OVERHAT])
        op->appendShort(equipment[Equipment::OVERHAT]->getEqpApr());
    else if (equipment[Equipment::HEAD])
        op->appendShort(equipment[Equipment::HEAD]->getEqpApr());
    else
        op->appendShort(hairstyle);
    if (!ghost) {
        unsigned char base = gender == 'm' ? 0x10 : 0x20;
        base += effects[StatusEffect::CONCEAL] ? 0x40 : 0;
        if (equipment[Equipment::ARMOR]) //10/20 normal 30/40 dead 50/60 hidden other weird options after
            op->appendByte(base | equipment[Equipment::ARMOR]->getExtraApr());
        else
            op->appendByte(base);
        op->appendByte(0);
        op->appendByte(1);
        if (equipment[Equipment::BOOTS])
            op->appendByte(equipment[Equipment::BOOTS]->getEqpApr()); //BOOTS
        else
            op->appendByte(0);
        if (equipment[Equipment::ARMOR])
            op->appendShort(equipment[Equipment::ARMOR]->getEqpApr());
        else
            op->appendShort(0);

        if (equipment[Equipment::SHIELD])
            op->appendByte(equipment[Equipment::SHIELD]->getEqpApr());
        else
            op->appendByte(0xFF);
        if (equipment[Equipment::WEAPON])
            op->appendShort(equipment[Equipment::WEAPON]->getEqpApr());
        else
            op->appendShort(0);
        op->appendByte(haircolor);
        if (equipment[Equipment::BOOTS])
            op->appendByte(equipment[Equipment::BOOTS]->getExtraApr());
        else
            op->appendByte(0);
        //Accessory
        if (equipment[Equipment::ACC]) {
            op->appendByte(1);
            op->appendShort(equipment[Equipment::ACC]->getEqpApr());
        }
        else {
            op->appendByte(0);
            op->appendShort(0);
        }
    }
    else { //Dead
        op->appendByte((gender == 'm' ? 0x30 : 0x40));
        op->appendShort(1);
        op->appendInt(0xFF);
        op->appendShort(0);
        op->appendByte(1); //ghosts have black hair
        op->appendByte(0); //boots
        op->appendByte(0); //acc not present
        op->appendShort(0); //acc
    }
    op->appendByte(0);
    op->appendInt(0);
    op->appendInt(0);
    op->appendInt(0);
    op->appendShort(0);
    op->appendString(name.length(), name.c_str());
    op->appendByte(0);
}

void Character::getStatBlock(IDataStream *dp, unsigned int flags)
{

    dp->appendByte((unsigned char) flags);

    if (flags & FLAG_MAX) {
        dp->appendShort(0x0100);
        dp->appendByte(0); //function of these two is unknown
        dp->appendByte(level);
        dp->appendByte(ab); //
        dp->appendInt(stats.getHp());
        dp->appendInt(stats.getMp());
        dp->appendByte(stats.getStr());
        dp->appendByte(stats.getInt());
        dp->appendByte(stats.getWis());
        dp->appendByte(stats.getCon());
        dp->appendByte(stats.getDex());
        dp->appendByte(statPoints > 0 ? 0x1F : 0); //1 seems to work as well, not sure why 1f used
        dp->appendByte(statPoints);
        dp->appendShort(maxWeight);
        dp->appendShort(curWgt()); //current weight
        dp->appendByte(stats.getCappedAc());
        dp->appendShort(0);
        dp->appendByte(0); //function of these two is unknown
    }
    if (flags & FLAG_HPMP) {
        dp->appendInt(hp);
        dp->appendInt(curMp);
    }
    if (flags & FLAG_POINTS) {
        dp->appendInt(exp);
        dp->appendInt(getTni());
        dp->appendInt(0); //ap
        dp->appendInt(0); //tna
        dp->appendInt(0); //gp
        dp->appendInt(gold);
    }
    if (flags & FLAG_SECONDARY) {
        dp->appendInt(0);
        dp->appendShort(0);
        dp->appendByte((char) getAtkEle());
        dp->appendByte((char) getDefEle());
        dp->appendByte(stats.getMr());
        dp->appendByte(1); //unknown function
        dp->appendByte(stats.getCappedAc());
        dp->appendByte(stats.getDmg());
        dp->appendByte(stats.getHit());
    }

}

int Character::getTni()
{
    if (level < 99)
        return (paths[path].levelMod * (level * (1 + (level * (3 + 2 * level))))
            / 6) - exp;
    return 0;
}

void Character::gainExp(unsigned int exp)
{
    char msg[24]; //10+1+12+1=24
    snprintf(msg, 24, "%u experience!", exp);
    Server::sendMessage(session, msg);
    this->exp = this->exp + exp > 0 ? this->exp + exp : this->exp;
    while (getTni() <= 0 && level < 99) {
        levelUp();
    }

    Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
    //Strictly speaking, the server does this first but will mess up the tni as a result

}

bool Character::takeExp(unsigned int amt)
{
    if (exp < amt)
        return false;

    exp -= amt;
    Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
    return true;
}

void Character::gainExp(unsigned int exp, int mobId)
{
    addKill(mobId);

    //Max exp is 10% of level if awarded from mobs
    exp =
        exp < (unsigned) (level * level * paths[path].levelMod) ?
            exp : level * level * paths[path].levelMod;
    exp = exp * config::expMod / 100;
    exp = exp > 0 ? exp : 1;
    gainExp(exp);
}

void Character::addKill(int mobId, int nKills)
{
    for (Tracker *t : trackers) {
        t->killed(mobId, nKills);
    }
}

void Character::addMaxHp(int amt)
{
    baseStats.incHp(amt);
    stats.incHp(amt);
    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);
}

void Character::addMaxMp(int amt)
{
    baseStats.incMp(amt);
    stats.incMp(amt);
    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);
}

unsigned int Character::countKills(int trackerId, int mobId)
{
    for (Tracker *t : trackers) {
        if (t->getQid() == trackerId) {
            return t->countId(mobId);
        }
    }

    return 0;
}

unsigned int Character::countAllKills(int trackerId)
{
    for (Tracker *t : trackers) {
        if (t->getQid() == trackerId) {
            return t->countTotal();
        }
    }

    return 0;
}

void Character::addTracker(int qid, int *mobList, int n, int *qty)
{
    //Ignore if existing
    for (Tracker *t : trackers) {
        if (t->getQid() == qid)
            return;
    }

    trackers.push_back(new Tracker(qid, mobList, n, qty));
}

void Character::deleteTracker(int qid)
{
    for (auto it = trackers.begin(); it != trackers.end(); it++) {
        if ((*it)->getQid() == qid) {
            delete (*it);
            trackers.erase(it);
            return;
        }
    }
}

void Character::incStat(char which)
{
    if (!statPoints) {
        Server::sendMessage(session, "Nothing happens.");
        return;
    }

    if (which == 1) { //STR
        if (baseStats.getStr() > paths[path].maxStr)
            Server::sendMessage(session, "You can't have any more.");
        else {
            --statPoints;
            incStr(1);
            Server::sendMessage(session, "Your muscles harden.");
        }
    }
    else if (which == 2) { //DEX
        if (baseStats.getDex() > paths[path].maxDex)
            Server::sendMessage(session, "You can't have any more.");
        else {
            --statPoints;
            incDex(1);
            Server::sendMessage(session, "You feel more nimble.");
        }
    }
    else if (which == 4) { //INT
        if (baseStats.getInt() > paths[path].maxInt)
            Server::sendMessage(session, "You can't have any more.");
        else {
            --statPoints;
            incInt(1);
            Server::sendMessage(session, "You understand more.");
        }
    }
    else if (which == 0x10) { //CON
        if (baseStats.getCon() > paths[path].maxCon)
            Server::sendMessage(session, "You can't have any more.");
        else {
            --statPoints;
            incCon(1);
            Server::sendMessage(session, "Energy flows into you.");
        }
    }
    else if (which == 0x08) { //WIS
        if (baseStats.getWis() > paths[path].maxWis)
            Server::sendMessage(session, "You can't have any more.");
        else {
            --statPoints;
            incWis(1);
            Server::sendMessage(session, "You feel more in touch.");
        }
    }

}

void Character::incStr(int amt)
{
    baseStats.incStr(amt);
    stats.incStr(amt);
    maxWeight += amt;
    Server::updateStatInfo(session, this, FLAG_SECONDARY | FLAG_MAX);
}
void Character::incDex(int amt)
{
    baseStats.incDex(amt);
    stats.incDex(amt);
    Server::updateStatInfo(session, this, FLAG_SECONDARY | FLAG_MAX);
}
void Character::incCon(int amt)
{
    baseStats.incCon(amt);
    stats.incCon(amt);
    Server::updateStatInfo(session, this, FLAG_SECONDARY | FLAG_MAX);
}
void Character::incWis(int amt)
{
    baseStats.incWis(amt);
    stats.incWis(amt);
    Server::updateStatInfo(session, this, FLAG_SECONDARY | FLAG_MAX);
}
void Character::incInt(int amt)
{
    baseStats.incInt(amt);
    stats.incInt(amt);
    Server::updateStatInfo(session, this, FLAG_SECONDARY | FLAG_MAX);
}

void Character::levelUp()
{
    ++level;
    //max hp/mp are increased by 20<>30 + 50con_wis/newLevel
    std::uniform_int_distribution<int> hpmp_dist(0, 10);
    int hpGain = 20 + baseStats.getCon() * 50 / level + hpmp_dist(generator());
    int mpGain = 20 + baseStats.getWis() * 50 / level + hpmp_dist(generator());
    baseStats.incHp(hpGain);
    stats.incHp(hpGain);
    baseStats.incMp(mpGain);
    stats.incMp(mpGain);

    //ac increased for every 3 levels
    if (level % 3 == 0) {
        baseStats.incAc(-1);
        stats.incAc(-1);
    }

    //add 2 stat points
    statPoints += 2;

    Server::sendMessage(session, "Your level has increased.");
    playEffect(0x4f, 0x64);
    Server::updateStatInfo(session, this,
        FLAG_MAX | FLAG_HPMP | FLAG_SECONDARY);

}

bool Character::tryMove(char dir)
{
    // If on a world map, don't move
    if (f)
        return false;

    // Check move limit
    if (walkLimit < 2)
        return false;

    cancelCasting();
    bool r = Entity::tryMove(dir);
    if (!r)
        return false;
    Map *m = getMap();
    Portal *p = m->getPortal(getX(), getY());
    if (p) {
        f = p->getField();
        if (f) {
            //Field warps shouldn't notify the player that they are changing maps
            DataService::getService()->tryChangeMap(this, GATE, 5, 5, 0);
            Server::fieldWarp(session, f->name, f->dests);
        }
        else if (!DataService::getService()->tryChangeMap(this, p->getMapId(),
							  p->destX(),
							  p->destY(), 2)) {
	    //Uh oh, this portal is dead
	    LOG4CPLUS_WARN(core::log(),
			   "Portal to map " << p->getMapId() <<
			   " is pointing at " << "a non-existent map.");
        }
    }
    if (r)
        walkLimit -= 2;
    return r;
}

short Character::countFreeSlots()
{
    return inventory.getFreeSlots();
}

void Character::fieldJump(short mapId)
{
    //On a field?
    if (!f)
        return;
    for (auto it = f->dests.begin(); it != f->dests.end(); it++) {
        if (it->mapId == mapId) {
            //This jump is allowed

            if (!DataService::getService()->tryChangeMap(this, mapId, it->xdest,
                it->ydest, 4))
                DataService::getService()->tryChangeMap(this, mapId, it->xdest,
                    it->ydest, 0);

            f = 0;
            return;
        }
    }
}

void Character::alive()
{
    ghost = false;
    hp = 1;
    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
}

void Character::die()
{
    hp = 1;
    curMp = 0;
    Server::sendMessage(session, "You have died.");
    Map *m = getMap();

    if (!m->pvpOn()) {

        if (level < 99)
            exp = exp - getTni() / 5 > 0 ? exp - getTni() / 5 : 0;
        else if (baseStats.getHp() > 50) {
            Server::sendMessage(session, "Your vitality has decreased by 50.");
            baseStats.incHp(-50);
            stats.incHp(-50);
        }

        //drop perishable items
        std::vector<unsigned int> protection { (unsigned) charId }; //protect these items
		auto protectionTime = std::chrono::minutes(30);
        for (int i = 0; i < NUM_ITEMS; i++) {
            if (inventory[i]
                && inventory[i]->getDeathAction() == Item::PERISH) {
                Item *lost = inventory.remove(i, this);
                if (lost)
                    if (!m->putItem(lost, getX(), getY(), &protection,
                        protectionTime))
                        lostItems.push_back(lost);
            }
        }

        //drop or perish equipped gear
        for (int i = 1; i < Equipment::Slots::ACC; i++) {
            if (equipment[i]) {
                Equipment *eq;
                switch (equipment[i]->getDeathAction()) {
                case (Item::PERISH):
                    if (config::itemsPerish) {
                        eq = removeEquipment(i);
                        delete eq;
                    }
                    else if (config::itemsDrop) {
                        eq = removeEquipment(i);
                        if (!m->putItem(eq, getX(), getY(), &protection,
                            protectionTime))
                            lostItems.push_back(eq);
                    }
                    break;
                case (Item::DROP):
                    if (config::itemsDrop) {
                        eq = removeEquipment(i);
                        if (!m->putItem(eq, getX(), getY(), &protection,
                            protectionTime))
                            lostItems.push_back(eq);
                    }
                    break;
                default:
                    //TODO unequip prevents the gear from being destroyed if no
                    //slots are available, but it generates unnecessary network traffic here
                    unequip(i);
                    break;
                }
            }
        }

        //Drop money
        if (config::goldDrops
            && m->putGold(gold, getX(), getY(), &protection, protectionTime))
            gold = 0;

        ghost = true;
        Server::updateStatInfo(session, this, FLAG_ALL);
    }

    else { //In a PVP zone, no penalty
        ghost = true;
    }
}

bool Character::tick(std::vector<std::function<void()> > &deferred)
{
    //Update limits
    if (limitTicks >= TICKS) {
        limitTicks = 0;
        secretLimit = 0;
        itemLimit = 0;
        refreshed = false;
    }
    else if (secretLimit || itemLimit || refreshed) {
        limitTicks++;
    }
    walkLimit = (std::min)(walkLimit + 3, 6);

    //reduce cds
    for (int i = 0; i < NUM_SKILLS; i++) {
        if (skills[i])
            skills[i]->decCd();
        if (secrets[i])
            secrets[i]->decCd();
    }

    //Increment casting "bar"
    if (chantSet) {
        if (chantCount > chantSet) {
            //Chant resets
            chantCount = chantSet = 0;
            chantFree = 1;
        }
        else {
            if (chantCount == chantSet)
                chantFree = 1;
            chantCount++;
        }
    }
    else {
        chantFree = 1;
    }

    //Check natural regen timers
    if (++regenCount >= getRegenTimer()) {
        regen();
        regenCount = 0;
    }
    if (++mpRegenCount >= getMpRegenTimer()) {
        mpRegen();
        mpRegenCount = 0;
    }

    //Look for change in effect durations
    //TODO improve
    unsigned char olddur[StatusEffect::SE_KINDS];

    bool wasSkulled = false, notSkulled = false;
    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i]) {
            olddur[i] = effects[i]->getDur();
            if (i == StatusEffect::DOOM)
                wasSkulled = true;
        }
    }

    Entity::tick(deferred);

    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i] && olddur[i] != effects[i]->getDur())
            Server::sendStatusEffect(session, effects[i]->getIcon(),
                effects[i]->getDur());
        if (!effects[i] && i == StatusEffect::DOOM)
            notSkulled = true;
    }

    //Check for player dead
    bool dead = wasSkulled && notSkulled;
    if (!hp || dead) {
        if (group && !dead) {
            this->addStatusEffect(SE_SKULLED);
            hp = 1;
        }
        else {
            die();
            deferred.push_back([=]() {
                Server::leaveMap(this->session);
                Map *m = DataService::getService()->getMap(435);
                this->setX(10);
                this->setY(8);
                this->setMap(m);
                m->lock();
                m->tryFindFree(this, 3);
                changedMap(m);
                m->addEntity(this);
                m->unlock();
            });
            return true;
        }
    }

    return false;
}

void Character::changedMap(Map *newmap)
{
    const char *mapName = newmap->getName();
    Server::enterMap(session, newmap->getDispId(), newmap->getWidth(),
        newmap->getHeight(), newmap->getCrc(), mapName, strlen(mapName));
    if (newmap->getBgm())
        Server::setBgm(session, newmap->getBgm());

    //Show doors
    std::vector<Door *> nearDoors;
    newmap->getNearbyDoors(this, nearDoors);
    Server::sendDoors(session, nearDoors);

    //Don't know what this is for
    Server::send0x58(session);

    if (newmap->pvpOn())
        Server::sendMessage(session, "You may attack other players here.");
}

/**
 * \brief Gets the time per regeneration for this character
 *
 * Gets the timer per regeneration for this character. The time is in seconds, which depends on
 * the tick interval also being in seconds
 * \return The time interval, in seconds, after which the character should regenerate naturally
 */
int Character::getRegenTimer()
{
    //Regen time depends on con and is ~7 seconds at best. At worst it's about 20 seconds
    int bonus = stats.getCon() - level;
    bonus = bonus > 0 ? (bonus * TICKS) / 12 : 0;
    return (TICKS * 20 - bonus);
}

/**
 * \brief Causes the character to regenerate hp
 *
 * Causes the character to regenerate hp. Hp regen quantity is between 10 and 20 depending on con,
 * and then further modified by regen bonus (each point of regen bonus is .1% regen)
 */
void Character::regen()
{
    if (ghost || effects[StatusEffect::DOOM])
        return;
    int regen = 150;
    int mod = stats.getCon() - level;
    if (mod > 0)
        regen += mod >= 10 ? 50 : mod * 5;
    else
        regen += mod <= -10 ? -50 : mod * 5;

    regen += stats.getRegen();
    hp = hp + regen * stats.getHp() / 1000;
    if (hp > stats.getHp())
        hp = stats.getHp();

    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);

}

unsigned short Character::getDispHp()
{
    return hp * 100 / stats.getHp();
}

int Character::getMpRegenTimer()
{
    int bonus = stats.getWis() - level;
    bonus = bonus > 0 ? (bonus * TICKS) / 12 : 0;
    return (TICKS * 30 - bonus);
}

void Character::mpRegen()
{
    if (ghost || effects[StatusEffect::DOOM])
        return;
    int regen = 150;
    int mod = stats.getWis() - level;
    if (mod > 0)
        regen += mod >= 10 ? 50 : mod * 5;
    else
        regen += mod <= -10 ? -50 : mod * 5;

    curMp = curMp + regen * stats.getMp() / 1000;
    if (curMp > stats.getMp())
        curMp = stats.getMp();

    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
}

void Character::addSkill(int id, short level, short slot, int uses, unsigned cd)
{
    assert(!skills[slot]);

    char showSlot = getShowSlot(slot);
    assert(!secrets[slot]);

    skills[slot] = new Skill((unsigned int) id, getPath(), (char) level, uses);
    Server::getSkill(session, skills[slot], showSlot);

    if (cd) {
        skills[slot]->setCd(cd);
        Server::startCd(session, 1, showSlot, cd);
    }
}

void Character::addSecret(int id, short level, short slot, int uses,
    unsigned cd)
{
    char showSlot = getShowSlot(slot);
    assert(!secrets[slot]);

    secrets[slot] = new Secret(id, getPath(), level, uses);
    int weapon =
        equipment[Equipment::WEAPON] ?
            equipment[Equipment::WEAPON]->getId() : 0;
    Server::getSecret(session, secrets[slot], showSlot, weapon);

    if (cd) {
        secrets[slot]->setCd(cd);
        Server::startCd(session, 0, showSlot, cd);
    }
}

/**
 * \brief Add a legend item to the character
 *
 * Adds a legend item to the character. The item to be added will be appended.
 *
 * \param icon The icon for the item to be shown
 * \param color The color of the text to be shown
 * \param prefix The prefix(undisplayed) for this legend item.
 * \param text The text for this legend item.
 *
 */
void Character::addLegendItem(int id, const char *textParam, int intParam,
    int timestamp)
{
    //Check if updating
    for (LegendItem &li : legend) {
        if (li.base->getId() == id) {
            //overwrite
            li.intParam = intParam;
            strcpy(li.textParam, textParam);
            li.timestamp = timestamp;
            li.textlen = li.base->getText(li.text, 128, textParam, intParam,
                li.timestamp);
            if (updateDb)
                DataService::getService()->updateLegend(charId, id, textParam,
                    intParam, timestamp);
            return;
        }
    }

    //Not found
    LegendItem li;
    li.base = Legend::getById(id);
    if (!li.base) {
        LOG4CPLUS_ERROR(log(),
            "Failed to add legend ID " << id << " to character " << name << ": legend not found.");
        return;
    }

    li.intParam = intParam;
    strcpy(li.textParam, textParam);
    li.textlen = li.base->getText(li.text, 128, textParam, intParam, timestamp);
    li.timestamp = timestamp;
    legend.push_back(li);
    if (updateDb)
        DataService::getService()->addLegend(charId, id, textParam, intParam,
            timestamp);
}

void Character::addLegendItem(int id, const char *textParam, int intParam)
{
    addLegendItem(id, textParam, intParam, time(NULL));
}

void Character::changeLegendQty(int id, int intParam)
{
    for (auto it = legend.begin(); it != legend.end(); it++) {
        if (it->base->getId() == id) {
            if (it->intParam + intParam < 0) {
                legend.erase(it);
                if (updateDb)
                    DataService::getService()->deleteLegend(charId, id);
            }
            else {
                it->intParam += intParam;
                it->textlen = it->base->getText(it->text, 128, it->textParam,
                    it->intParam, time(NULL));
                if (updateDb)
                    DataService::getService()->updateLegend(charId, id,
                        it->textParam, it->intParam, it->timestamp);
            }
            return;
        }
    }

    LegendItem li;
    li.base = Legend::getById(id);
    if (!li.base) {
        LOG4CPLUS_ERROR(log(),
            "Failed to add legend ID " << id << " to character " << name << ": legend not found.");
        return;
    }

    li.intParam = intParam;
    li.textParam[0] = 0;
    li.timestamp = time(NULL);
    li.textlen = li.base->getText(li.text, 128, "", intParam, li.timestamp);
    legend.push_back(li);
    if (updateDb)
        DataService::getService()->addLegend(charId, id, NULL, intParam,
            li.timestamp);
}

void Character::changeLegendQty(int id, int intParam, const char *textParam)
{
    //TODO this method can probably be rewritten to call the one it overloads or v.v.
    for (auto it = legend.begin(); it != legend.end(); it++) {
        if (it->base->getId() == id) {
            if (it->intParam + intParam < 0) {
                legend.erase(it);
                if (updateDb)
                    DataService::getService()->deleteLegend(charId, id);
            }
            else {
                it->intParam += intParam;
                strcpy(it->textParam, textParam);
                it->textlen = it->base->getText(it->text, 128, it->textParam,
                    it->intParam, time(NULL));
                if (updateDb)
                    DataService::getService()->updateLegend(charId, id,
                        textParam, it->intParam, it->timestamp);
            }
            return;
        }
    }

    LegendItem li;
    li.base = Legend::getById(id);
    if (!li.base) {
	LOG4CPLUS_ERROR(log(),
            "Failed to add legend ID " << id << " to character " << name << ": legend not found.");
        return;
    }

    li.intParam = intParam;
    strcpy(li.textParam, textParam);
    li.timestamp = time(NULL);
    li.textlen = li.base->getText(li.text, 128, li.textParam, intParam,
        li.timestamp);
    legend.push_back(li);
    if (updateDb)
        DataService::getService()->addLegend(charId, id, textParam, intParam,
            li.timestamp);
}

void Character::removeLegend(int id)
{
    for (auto it = legend.begin(); it != legend.end(); it++) {
        if (it->base->getId() == id) {
            legend.erase(it);
            if (updateDb)
                DataService::getService()->deleteLegend(charId, id);
            return;
        }
    }
}

void Character::eqChangeMessage(Equipment *eq, Equipment::Slots slot)
{
    char buffer[128];
    if (eq) {
        const char *front;
        switch (slot) {
        case Equipment::ARMOR:
            front = "a:Armor :";
            break;
        case Equipment::SHIELD:
            front = "s:Shield :";
            break;
        case Equipment::R_RING:
            front = "r:RHand :";
            break;
        case Equipment::L_RING:
            front = "l:LHand :";
            break;
        case Equipment::L_HAND:
            front = "L:LArm :";
            break;
        case Equipment::R_HAND:
            front = "R:RArm :";
            break;
        case Equipment::NECKLACE:
            front = "n:Necklace :";
            break;
        case Equipment::HEAD:
            front = "h:Helmet :";
            break;
        case Equipment::EARRING:
            front = "e:Earring :";
            break;
        case Equipment::BELT:
            front = "b:Waist :";
            break;
        case Equipment::GREAVES:
            front = "g:Legs :";
            break;
        case Equipment::BOOTS:
            front = "f:Foot :";
            break;
        case Equipment::ACC:
            front = "M:Overcoat :";
            break;
        case Equipment::OVERHAT:
            front = "u:Coat :";
            break;
        default:
            front = "";
            break;
        }
        snprintf(buffer, 128, "%s%s Armor class %hd %hd S %hd", front,
            eq->getName(), stats.getAc(), stats.getRegen(), stats.getMr());
    }
    else {
        snprintf(buffer, 128, " Armor class %hd %hd S %hd", stats.getAc(),
            stats.getRegen(), stats.getMr());
    }
    Server::sendMessage(session, buffer);
}

/**
 * \brief Attempts to use the item in the slot given
 *
 * Attempts to use the item in the slot given. If the item can be used in some way,
 * the results take effect before this returns.
 * \param slot The slot of the item to be used, as was presented to the user (ie. in 1..59)
 */
void Character::useItem(char slot)
{
    cancelCasting();
    if (!canAct())
        return;
    Equipment *eq, *tmp;
    unsigned short newWgt;
    int eqpSlot;
    bool updSpells;

    int actSlot = slot - 1;
    if (actSlot < 0 || actSlot >= NUM_ITEMS || !inventory[actSlot])
        return;

    //Check item use limit
    if (itemLimit > ITEM_LIMIT) {
        return;
    }
    itemLimit++;

    switch (inventory[actSlot]->getType()) {
    case BaseItem::PLAIN:
        Server::sendMessage(session, "You can't use that.");
        break;
    case BaseItem::EQUIP:
        eq = (Equipment *) inventory[actSlot];
        newWgt = eqpWgt + eq->getWeight();
        if (!eq->allowsPath(path)) {
            Server::sendMessage(session,
                "Your path has forbidden itself from this vulgar implement.");
            break;
        }
        else if (!eq->allowsGender(gender)) {
            Server::sendMessage(session, "This doesn't fit you.");
            break;
        }
        else if (eq->levelReq() > effectiveLevel()) {
            Server::sendMessage(session, "You can't use this yet.");
            break;
        }
        //no boots onto monk armor
        else if (eq->getSlot() == Equipment::BOOTS
            && equipment[Equipment::ARMOR]
            && (equipment[Equipment::ARMOR]->getFlags() & 1)) {
            //what the heck does this even mean? thankfully sane people use shoes and clothes simultaneously
            Server::sendMessage(session,
                "Clothes and shoes can't be armoured at the same time.");
            break;
        }
        //no monk armor onto boots
        else if (eq->getSlot() == Equipment::ARMOR && (eq->getFlags() & 1)
            && equipment[Equipment::BOOTS]) {
            Server::sendMessage(session,
                "Clothes and shoes can't be armoured at the same time.");
            break;
        }
        //no shield and 2handed weapons
        else if (eq->getSlot() == Equipment::WEAPON
            && equipment[Equipment::SHIELD] && (eq->getFlags() & 4)) {
            Server::sendMessage(session, "Two-handed weapon.");
            break;
        }
        else if (eq->getSlot() == Equipment::SHIELD
            && equipment[Equipment::WEAPON]
            && (equipment[Equipment::WEAPON]->getFlags() & 4)) {
            Server::sendMessage(session, "Two-handed weapon.");
            break;
        }

        eqpSlot = eq->getSlot();
        if (equipment[eqpSlot]
            && (eqpSlot == Equipment::L_RING || eqpSlot == Equipment::L_HAND))
            ++eqpSlot;
        if (equipment[eqpSlot])
            newWgt -= equipment[eqpSlot]->getWeight();

        if (newWgt >= maxWeight >> 1) {
            Server::sendMessage(session, "It's too heavy.");
            break;
        }
        //Passes all requirements to equip
        eq = (Equipment *) inventory.remove(actSlot, this);
        assert(eq);

        //TODO worth using this check?
        updSpells = eq->getSlot() == Equipment::WEAPON;
        tmp = equipment[eqpSlot];
        if (tmp)
            stats -= *tmp->getStats();
        stats += *eq->getStats();
        equipment[eqpSlot] = eq;
        eqpWgt = newWgt;

        Server::equipItem(session, eqpSlot, eq->getName(), eq->getNameLen(),
            eq->getApr(), eq->getDur(), eq->getMaxDur());
        appearanceChanged();
        Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);

        if (tmp)
            inventory.addItem(tmp, this);

        if (updSpells) {
            for (int i = 0; i < NUM_SKILLS; i++) {
                if (secrets[i]) {
                    char slot;
                    if (i < 35)
                        slot = i + 1;
                    else if (i < 70)
                        slot = i + 2;
                    else
                        slot = i + 3;
                    Server::getSecret(session, secrets[i], slot, eq->getId());
                }
            }
        }
        if (eqpSlot != Equipment::WEAPON) {
            eqChangeMessage(eq, (Equipment::Slots) eqpSlot);
        }

        break;
    case BaseItem::CONSUME:
        Consumable *consume = (Consumable *) inventory[actSlot];
        if (consume->use(this)) {
            delete inventory.removeSome(actSlot, 1, this);
            Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
        }
    }
}

void Character::talked(Entity *who, std::string text, unsigned char channel)
{
    if (!channel) //talking channel
        text = who->getName() + (": " + text);
    int len = text.length();
    Server::talked(getSession(), who->getOid(), channel, text.c_str(), len);
}

void Character::showViewable(Viewable *v)
{
    if (v == (Viewable *) this)
        Server::setCoords(session, getX(), getY());
    else if (!canSee(v))
        return;
    Server::showViewable(session, v);
}

void Character::showViewables(const std::vector<Viewable *> &who)
{
    std::vector<Viewable *> seen;
    std::for_each(who.begin(), who.end(), [&](Viewable *v) {
        if (canSee(v))
        seen.push_back(v);
    });
    Server::showViewables(session, seen);
}

void Character::gearStruck()
{
    if (this->hasEffect(StatusEffect::INVULNERABILITY))
        return;

    for (int i = 2; i < Equipment::Slots::ACC; i++) {
        if (equipment[i]) {
            equipment[i]->decDur();
            if (equipment[i]->getDur() <= 0) {
                //broken
                stats -= *(equipment[i]->getStats());
                eqpWgt -= equipment[i]->getWeight();
                Server::sendMessage(session, "Broken.");
                Server::removeEquip(session, i);
                delete equipment[i];
                equipment[i] = 0;
                appearanceChanged();
                Server::updateStatInfo(session, this,
                    FLAG_MAX | FLAG_SECONDARY);
            }
            else {
                Server::equipItem(session, i, equipment[i]->getName(),
                    equipment[i]->getNameLen(), equipment[i]->getApr(),
                    equipment[i]->getDur(), equipment[i]->getMaxDur());
                int percent = 0;
                if (equipment[i]->getDur() == equipment[i]->getMaxDur() / 2) {
                    percent = 50;
                }
                else if (equipment[i]->getDur()
                    == equipment[i]->getMaxDur() / 4) {
                    percent = 25;
                }
                else if (equipment[i]->getDur()
                    == equipment[i]->getMaxDur() / 10) {
                    percent = 10;
                }
                if (percent) {
                    char buffer[100];
                    snprintf(buffer, 100, "%s is at %d%%",
                        equipment[i]->getName(), percent);
                    Server::sendMessage(session, buffer);
                }
            }
        }
    }
}

int Character::repairAllCost()
{
    int cost = inventory.repairCost();

    bool allRepaired = cost == -1;
    cost = 0;
    for (int i = 0; i < NUM_EQUIPS; i++) {
        if (equipment[i]) {
            allRepaired = !equipment[i]->canRepair() && allRepaired;
            cost += equipment[i]->repairCost();
        }
    }

    return allRepaired ? -1 : cost;
}

bool Character::repairAll()
{
    for (int i = 0; i < NUM_EQUIPS; i++) {
        if (equipment[i]
            && equipment[i]->getDur() < equipment[i]->getMaxDur()) {
            if (gold > equipment[i]->repairCost()) {
                gold -= equipment[i]->repairCost();
                equipment[i]->repair();
                Server::equipItem(session, i, equipment[i]->getName(),
                    equipment[i]->getNameLen(), equipment[i]->getApr(),
                    equipment[i]->getDur(), equipment[i]->getMaxDur());
            }
            else {
                Server::updateStatInfo(session, this,
                    FLAG_POINTS | FLAG_SECONDARY);
                return false;
            }
        }
    }

    bool r = inventory.repairAll(gold, this);
    Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
    return r;
}

/**
 * \brief Tries to acquire the item given
 *
 * Tries to acquire the item given. Can fail if weight or storage limits exceeded.
 * \param item [in] The item to acquire
 * \return True if the item was acquired, false otherwise
 */
bool Character::getItem(Item *item)
{
    if (curWgt() + item->getWeight() > maxWeight)
        return false;

    return inventory.addItem(item, this);
}

Item *Character::removeItem(int slot)
{
    if (slot < 0 || slot >= NUM_ITEMS)
        return 0;

    return inventory.remove(slot, this);
}

Item *Character::removeItem(int slot, int amt)
{
    if (slot < 0 || slot >= NUM_ITEMS)
        return 0;

    return inventory.removeSome(slot, amt, this);
}

void Character::addedItem(Item *item, int slot)
{
    Server::getItem(session, slot + 1, item->getQty(), item->getName(),
        item->getNameLen(), item->getApr(), item->getDur(), item->getMaxDur());

    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);

    if (updateDb)
        DataService::getService()->updateItem(charId, slot, item->getId(),
            item->getQty(), item->getDur(), item->getMod(), 0);
}

void Character::removedItem(int slot)
{
    Server::removeItem(session, slot + 1);
    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);

    if (updateDb)
        DataService::getService()->removeItem(charId, slot);
}

/**
 * \brief Puts the item in the slot given
 *
 * Puts the item in the slot given. putItem is intended to be used during loading, so unlike getItem,
 * it cannot fail. This means it could overwrite the previous item or put the character overweight
 * \param item [in] The item which the character has acquired
 * \param slot [in] The slot to put the item. The slots 0..59 are inventory slots, while 60..72 are
 * 					equipment slots as enumerated in equipment.
 *
 * \sa Character::getItem
 */
void Character::putItem(Item *item, unsigned char slot)
{
    if (slot < NUM_ITEMS)
        inventory.putItem(item, slot, this);
    else {
        Equipment *eq = (Equipment *) item;
        slot -= NUM_ITEMS;
        if (equipment[slot]) {
            Server::removeEquip(session, slot);
            eqpWgt -= equipment[slot]->getWeight();
            stats -= *(equipment[slot]->getStats());
            delete equipment[slot];
        }
        equipment[slot] = eq;
        eqpWgt += eq->getWeight();
        stats += *(eq->getStats());
        Server::equipItem(session, slot, eq->getName(), eq->getNameLen(),
            eq->getApr(), eq->getDur(), eq->getMaxDur());
    }
}

void Character::reload()
{
    //Send items, buffs, and finally skills
    for (int i = 0; i < NUM_ITEMS; i++) {
        if (inventory[i]) {
            addedItem(inventory[i], i);
        }
    }

    for (int i = 0; i < NUM_EQUIPS; i++) {
        if (equipment[i]) {
            Server::equipItem(session, i + 1, equipment[i]->getName(),
                equipment[i]->getNameLen(), equipment[i]->getApr(),
                equipment[i]->getDur(), equipment[i]->getMaxDur());
        }
    }

    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (effects[i]) {
            Server::sendStatusEffect(session, effects[i]->getIcon(),
                effects[i]->getDur());
            Server::sendMessage(session, effects[i]->getReceivedMsg());
            Server::updateStatInfo(session, this, FLAG_ALL);
        }
    }

    for (int i = 0; i < NUM_SKILLS; i++) {
        char showSlot = getShowSlot(i);
        if (skills[i])
            Server::getSkill(session, skills[i], showSlot);
        if (secrets[i])
            Server::getSecret(session, secrets[i], showSlot,
                equipment[Equipment::WEAPON] ?
                    equipment[Equipment::WEAPON]->getId() : 0);
    }
}

/**
 * \brief Unequip the item in the given slot
 *
 * Unequips the item in the slot given. If the item cannot be acquired, it will not be unequipped.
 * \param slot [in] The slot of the item to unequip
 */
void Character::unequip(char slot)
{
    if (!canAct())
        return;
    cancelCasting();
    int i_slot = slot;
    if (i_slot < 0 || i_slot > NUM_EQUIPS)
        return;
    if (!equipment[i_slot])
        return;

    //Try to put in inventory
    eqpWgt -= equipment[i_slot]->getWeight();
    if (!inventory.addItem(equipment[i_slot], this)) {
        eqpWgt += equipment[i_slot]->getWeight();
        return;
    }

    //success
    stats -= *(equipment[i_slot]->getStats());
    equipment[i_slot] = 0;
    appearanceChanged();

    if (i_slot == Equipment::WEAPON) {
        for (int i = 0; i < NUM_SKILLS; i++) {
            char slot;
            if (i < 35)
                slot = i + 1;
            else if (i < 70)
                slot = i + 2;
            else
                slot = i + 3;
            if (secrets[i])
                Server::getSecret(session, secrets[i], slot, 0);
        }
    }
    else {
        eqChangeMessage(0, (Equipment::Slots) i_slot);
    }

    Server::removeEquip(session, slot);
    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);

}

Equipment *Character::removeEquipment(int slot)
{
    assert(equipment[slot]);
    unsigned short weight = equipment[slot]->getWeight();
    Equipment *r = equipment[slot];
    eqpWgt -= weight;
    stats -= *(r->getStats());
    equipment[slot] = 0;
    Server::removeEquip(session, slot);
    Server::updateStatInfo(session, this, FLAG_MAX | FLAG_SECONDARY);
    return r;
}

void Character::dropItem(char slot, unsigned short x, unsigned short y,
    unsigned int num)
{
    unsigned int actSlot = slot - 1;
    if (actSlot >= NUM_ITEMS)
        return;

    //Test if close enough
    unsigned short delta = x > getX() ? x - getX() : getX() - x;
    if (delta > 2)
        return;
    delta = y > getY() ? y - getY() : getY() - y;
    if (delta > 2)
        return;

    if (inventory[actSlot] && inventory[actSlot]->isBound()) {
        Server::sendMessage(session, "You can't drop this.");
        return;
    }

    Item *drop = inventory.removeSome(actSlot, num, this);
    if (!drop)
        return;

    //Triggers
    std::vector<Trigger *> triggers;
    getMap()->getTriggers(triggers, x, y);
    for (Trigger *t : triggers) {
        if ((*t)(drop, this)) {
            delete drop;
            return;
        }
    }

    if (!getMap()->putItem(drop, x, y)) {
        Server::sendMessage(session, "You can't drop this here.");
        if (!inventory.addItem(drop, this)) {
            lostItems.push_back(drop);
        }
    }
}

/**
 * \brief Increase this character's mp by a specified amount.
 *
 * Changes the character's mp by a specified amount. If that amount is
 * negative, its value is taken from the character's current mp.
 * The character's current mp will never exceed their maximum, and never drop
 * below 0.
 * \param[in] mp The amount of mp to be added to the player's current mp.
 */
void Character::addMp(int mp)
{
    curMp = CLIP(curMp + mp, 0, stats.getMp());
    Server::updateStatInfo(session, this, FLAG_HPMP | FLAG_SECONDARY);
}

void Character::dropGold(unsigned int amt, unsigned short x, unsigned short y)
{
    if (gold < amt)
        return;

    if (!getMap()->putGold(amt, x, y)) {
        Server::sendMessage(session, "You can't drop this here.");
        return;
    }

    gold -= amt;
    Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
}

bool Character::repair(unsigned int slot)
{
    bool r = inventory.repair(slot, gold, this);
    if (r)
        Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
    return r;
}

void Character::pickupItem(char slot, unsigned short x, unsigned short y)
{
    if (DIFF(x,getX()) > 2 || DIFF(y, getY()) > 2)
        return;

    GroundItem *item = getMap()->getItem(x, y, charId);
    if (!item) {
        Server::sendMessage(session, "You can't pick that up.");
        return;
    }

    if (item->isGold()) {
        gold += item->getGoldAmt();
        if (gold > MAX_GOLD) {
            //drop extra
            unsigned int extra = gold - MAX_GOLD;
            getMap()->putGold(extra, getX(), getY());
            gold = MAX_GOLD;
            Server::sendMessage(session, "You cannot carry any more gold.");
        }
        Server::updateStatInfo(session, this, FLAG_POINTS | FLAG_SECONDARY);
    }

    else if (item->getItem()->getWeight() + curWgt() > maxWeight) {
        Server::sendMessage(session, "It's too heavy.");
        if (!getMap()->putItem(item->getItem(), getX(), getY()))
            if (!getMap()->putItem(item->getItem(), x, y))
                lostItems.push_back(item->getItem());
    }
    else if (!inventory.addItem(item->getItem(), this)) {
        Server::sendMessage(session, "You can't have any more.");
        if (!getMap()->putItem(item->getItem(), getX(), getY()))
            if (!getMap()->putItem(item->getItem(), x, y))
                lostItems.push_back(item->getItem());
    }

    delete item;
}

unsigned short Character::countItems(int id, unsigned short mod)
{
    return inventory.countItems(id, mod);
}

bool Character::withdraw(int slot, int qty)
{
    if (slot > (signed) storage.size()) {
        return false;
    }

    BaseItem *bi = BaseItem::getById(storage[slot].id);
    if (!bi) {
        LOG4CPLUS_ERROR(log(),
            "Item ID " << storage[slot].id << " in the storage of player " << name << " doesn't identify an item.");
        return false;
    }

    qty = qty ? qty : 1;
    if (qty > storage[slot].qty)
        return false;

    Item *itm = new Item(bi);
    if (bi->getMaxStack()) {
        itm->setQty(qty);
    }
    if (storage[slot].mod) {
        ((Equipment *) itm)->setMod(storage[slot].mod);
    }

    //Try to get the item
    if (!inventory.addItem(itm, this)) {
        delete itm;
        return false;
    }

    DataService::getService()->updateStoredItem(charId, storage[slot].id,
        storage[slot].qty - qty, storage[slot].mod);

    if (storage[slot].qty == qty) {
        storage[slot] = storage[storage.size() - 1];
        storage.pop_back();
    }
    else
        storage[slot].qty -= qty;

    return true;
}

void Character::setStored(int itemId, int qty, short mod)
{
    StorageItem ni;
    ni.id = itemId;
    ni.qty = qty;
    ni.mod = mod;

    storage.push_back(ni);
}

bool Character::deposit(Item *itm)
{
    //Can't deposit a damaged item
    if (itm->getDur() != itm->getMaxDur()) {
        return false;
    }

    bool found = false;
    for (StorageItem &si : storage) {
        if (si.id == itm->getId() && si.mod == itm->getMod()) {
            //Add to existing stack
            if (itm->getMaxQty())
                si.qty += itm->getQty();
            else
                ++(si.qty);
            found = true;
            DataService::getService()->updateStoredItem(charId, si.id, si.qty,
                si.mod);
            break;
        }
    }

    if (!found) {
        //New stack
        StorageItem ni;
        ni.id = itm->getId();
        ni.mod = itm->getMod();
        ni.qty = itm->getMaxQty() ? itm->getQty() : 1;
        storage.push_back(ni);
        DataService::getService()->storeItem(charId, ni.id, ni.qty, ni.mod);
    }

    return true;
}

void Character::refresh()
{
    if (refreshed)
        return;
    DataService::getService()->leaveMap(this);
    DataService::getService()->enterMap(this);
    refreshed = true;
}

unsigned short Character::getItemQty(unsigned short slot)
{
    if (slot >= NUM_ITEMS || !inventory[slot])
        return 0;

    if (inventory[slot]->getMaxQty())
        return inventory[slot]->getQty();
    return 1;
}

void Character::setMaster()
{
    pathMask |= paths[Master].mask;
    if (updateDb)
        DataService::getService()->updatePath(charId, path, pathMask);
}

void Character::rededicate(Path p)
{
    assert(p != path);

    Stats diff = stats - baseStats;

    exp = 0;
    level = 1;
    setPath(p);

    statPoints = statPoints <= 19 ? statPoints : 19;
    baseStats.setStr(3);
    baseStats.setDex(3);
    baseStats.setCon(3);
    baseStats.setInt(3);
    baseStats.setWis(3);
    baseStats.setMaxHp(50);
    baseStats.setMaxMp(30);

    maxWeight = 51;

    stats = diff + baseStats;

    Server::updateStatInfo(session, this, FLAG_ALL);
}
