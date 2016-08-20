/*
 * CharManager.cpp
 *
 *  Created on: 2013-06-30
 *      Author: per
 */

#include "CharManager.h"
#include "DataLoaders.h"
#include <assert.h>
#include <algorithm>
#include "log4cplus/loggingmacros.h"
#include "lower.h"
#include <time.h>
#include <utility>

#ifdef WIN32
#include <WinSock2.h>
#define HAVE_STRUCT_TIMESPEC
#include <libpq-fe.h>
#include <Windows.h>
#define snprintf _snprintf
#define PATH_MAX MAX_PATH
#else
#include <postgresql/libpq-fe.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#endif

#include "LockSet.h"

template<typename T>
void doCopy(PGconn *conn, std::vector<T> &todos)
{
    char buffer[300];

    for (T &t : todos) {
        int len = t.copyString(buffer, 300);
#ifndef NDEBUG
        int n =
#endif
            PQputCopyData(conn, buffer, len);
        assert(n == 1);
    }

#ifndef NDEBUG
    int n =
#endif
        PQputCopyEnd(conn, NULL);
    assert(n == 1);
}

void updateChars(PGconn *conn, std::vector<Database::CharStats> &chars)
{
    //Truncate update table
    PGresult *r = PQexec(conn, "TRUNCATE TABLE update_character");
    if (PQresultStatus(r) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character data.");
        PQclear(r);
        return;
    }
    PQclear(r);

    r = PQexec(conn, "COPY update_character FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character data.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, chars);

    r = PQexec(conn, "UPDATE characters old SET "
        "mapid=new.mapid,x=new.x,y=new.y,hp=new.hp,mp=new.mp,"
        "maxhp=new.maxhp,maxmp=new.maxmp,strength=new.strength,"
        "intelligence=new.intelligence,wisdom=new.wisdom,"
        "dexterity=new.dexterity,constitution=new.constitution,"
        "statpoints=new.statpoints,level=new.level,"
        "ab=new.ab,exp=new.exp,ap=new.ap,gold=new.gold,"
        "dead=new.dead,labor=new.labor,"
        "laborreset=new.laborreset,last_login=new.last_login,"
        "stored_gold=new.stored_gold,accident_gold=new.accident_gold,"
        "settings=new.settings "
        "FROM update_character new "
        "WHERE old.id=new.id");
    if (PQresultStatus(r) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to finish copy operation for updating character data.");
        PQclear(r);
        return;
    }
    PQclear(r);
}

void insertItems(PGconn *conn, std::vector<Database::InsertEquip> &items)
{
    PGresult *r =
        PQexec(conn,
            "COPY has_item (char_id, item_id, slot, qty, dur, mod, identified) FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character equipment.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, items);
}

void insertEffects(PGconn *conn, std::vector<Database::Effect> &effects)
{
    PGresult *r = PQexec(conn,
        "COPY has_effect (char_id, buff_id, duration) FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character effects.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, effects);
}

void updateSkills(PGconn *conn, std::vector<Database::UpdateSkill> &skills)
{
    //Truncate update table
    PGresult *r = PQexec(conn, "TRUNCATE update_skill");
    if (PQresultStatus(r) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character skills.");
        PQclear(r);
        return;
    }
    PQclear(r);

    r = PQexec(conn, "COPY update_skill FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character skills.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, skills);

    r = PQexec(conn, "UPDATE has_skill old SET "
        "level=new.level,uses=new.uses,cd=new.cd "
        "FROM update_skill new "
        "WHERE old.char_id=new.char_id AND old.slot=new.slot");
    if (PQresultStatus(r) != PGRES_COMMAND_OK)
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to finish copy operation for updating character skills.");
    PQclear(r);
}

void updateSecrets(PGconn *conn, std::vector<Database::UpdateSkill> &secrets)
{
    //Truncate update table
    PGresult *r = PQexec(conn, "TRUNCATE update_secret");
    if (PQresultStatus(r) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character secrets.");
        PQclear(r);
        return;
    }
    PQclear(r);

    r = PQexec(conn, "COPY update_secret FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character secrets.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, secrets);

    r = PQexec(conn, "UPDATE has_secret old SET "
        "level=new.level,uses=new.uses,cd=new.cd "
        "FROM update_secret new "
        "WHERE old.char_id=new.char_id AND old.slot=new.slot");
    if (PQresultStatus(r) != PGRES_COMMAND_OK)
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to finish copy operation for updating character secrets.");
    PQclear(r);
}

void insertAccidentItems(PGconn *conn,
    std::vector<Database::AccidentItem> &items)
{
    PGresult *r =
        PQexec(conn,
            "COPY accident_item (char_id, item_id, qty, dur, mod, temp) FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character temporary items.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, items);
}

void insertTrackers(PGconn *conn, std::vector<Database::TrackerEntry> &trackers)
{
    PGresult *r = PQexec(conn,
        "COPY trackers (char_id, quest_id, mob_id, qty) FROM STDIN");
    if (PQresultStatus(r) != PGRES_COPY_IN) {
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to start copy operation for updating character trackers.");
        PQclear(r);
        return;
    }
    PQclear(r);

    doCopy(conn, trackers);
}

namespace Database
{

CharManager *cm;
template<typename T>
class DisableUpdates
{
public:
    DisableUpdates(T *t)
    {
        c = t;
        t->disableUpdates();
    }
    ~DisableUpdates()
    {
        c->enableUpdates();
    }
private:
    T *c;
};

CharManager::CharManager(const char *connParams)
{

    conn = PQconnectdb(connParams);
    bgconn = PQconnectdb(connParams);

	if (PQstatus(conn) != CONNECTION_OK || PQstatus(bgconn) != CONNECTION_OK) {
		if (PQstatus(conn) != CONNECTION_OK)
			LOG4CPLUS_FATAL(Database::log(), "Failed to connect to DB:" << PQerrorMessage(conn));
		else
			LOG4CPLUS_FATAL(Database::log(), "Failed to connect to DB:" << PQerrorMessage(bgconn));
		assert(false);
	}

    //Open new transaction
    PGresult *b = PQexec(conn, "BEGIN");
    if (PQresultStatus(b) != PGRES_COMMAND_OK) {
        LOG4CPLUS_FATAL(Database::log(), "Failed to restart batch save.");
        assert(false);
    }
    PQclear(b);
}

CharManager::~CharManager()
{
    //rollback, alternatively we could finish if we could save all chars
    PGresult *b = PQexec(conn, "ROLLBACK");
    if (PQresultStatus(b) != PGRES_COMMAND_OK) {
        LOG4CPLUS_FATAL(Database::log(), "Failed to rollback last transaction.");
        PQclear(b);
        assert(false);
        return;
    }
    PQclear(b);

    //Release outgoing characters
    for (Character *c : outgoing) {
        delete c;
    }

    //Join last save operation
    if (worker.joinable()) {
        LOG4CPLUS_INFO(Database::log(),
            "Waiting for last transaction to terminate.");
        worker.join();
        LOG4CPLUS_INFO(Database::log(),
            "Last transaction completed successfully.");
    }

    PQfinish(conn);
    PQfinish(bgconn);
}

void finishUpdate(std::vector<CharStats> &chars,
    std::vector<InsertEquip> &equips, std::vector<AccidentItem> &items,
    std::vector<UpdateSkill> &skills, std::vector<UpdateSkill> &secrets,
    std::vector<Effect> &effects, std::vector<TrackerEntry> &trackers,
    PGconn *conn)
{
    //Copy data
    updateChars(conn, chars);
    insertAccidentItems(conn, items);
    insertEffects(conn, effects);
    insertItems(conn, equips);
    updateSecrets(conn, secrets);
    updateSkills(conn, skills);
    insertTrackers(conn, trackers);

    //Commit
    PGresult *e = PQexec(conn, "COMMIT");
    if (PQresultStatus(e) != PGRES_COMMAND_OK) {
        LOG4CPLUS_FATAL(Database::log(), "Failed to commit batch save.");
        PQclear(e);
        assert(false);
        return;
    }
    PQclear(e);
}

struct UpdateParams
{
    std::vector<CharStats> chars;
    std::vector<InsertEquip> equips;
    std::vector<AccidentItem> items;
    std::vector<UpdateSkill> skills, secrets;
    std::vector<Effect> effects;
    std::vector<TrackerEntry> trackers;
    PGconn *conn;
};

void finishUpdWrapper(UpdateParams *params)
{
    finishUpdate(params->chars, params->equips, params->items, params->skills,
        params->secrets, params->effects, params->trackers, params->conn);
    delete params;
}

void CharManager::finish()
{
    if (worker.joinable())
    	worker.join();

    for (Character *c : outgoing) {
        saveCharacter(c);
        delete c;
    }

    outgoing.clear();

    UpdateParams *vects = new UpdateParams { std::move(chars), std::move(
        equips), std::move(items), std::move(skills), std::move(secrets),
        std::move(effects), std::move(trackers), conn };
	worker = std::thread(finishUpdWrapper, vects);
	if (!worker.joinable())
		LOG4CPLUS_ERROR(Database::log(), "Failed to create a thread for updating characters.");

    items.clear();
    effects.clear();
    equips.clear();
    chars.clear();
    secrets.clear();
    skills.clear();
    trackers.clear();

    //Open new transaction
    PGresult *b = PQexec(bgconn, "BEGIN");
    if (PQresultStatus(b) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Failed to restart batch save.");
    }
    PQclear(b);

    std::swap(conn, bgconn);
}

void CharManager::freeCharacter(Character *c)
{
    c->setSession(0);
    outgoing.push_back(c);
}

void CharManager::saveCharacter(Character *c)
{
    const int charId = c->getId();

    //TODO Need to get unclipped stats
    const Stats *stats = c->getBaseStats();
    CharStats st;
    st.map = c->getMap()->getDispId();	// c->getMapId();
    st.x = c->getX();
    st.y = c->getY();
    st.hp = c->getHp();
    st.mp = c->getMp();
    st.maxHp = stats->getHp();
    st.maxMp = stats->getMp();
    st.strength = stats->getStr();
    st.int_ = stats->getInt();
    st.wis = stats->getWis();
    st.con = stats->getCon();
    st.dex = stats->getDex();
    st.stPts = c->getStatPoints();
    st.lev = c->getLevel();
    st.ab = c->getAb();
    st.dead = c->isDead() ? 1 : 0;
    st.exp = c->getExp();
    st.ap = c->getAp();
    st.gold = c->getGold();
    c->getLaborCountTime(st.labor, st.laborReset);
    st.lastLogin = time(NULL);
    st.storedGold = c->getStoredGold();
    st.exchangeGold = c->getExchangeGold();
    st.settings = c->getSettings();
    st.id = charId;

    chars.push_back(std::move(st));

    //clear equipment
    int paramId = htonl(charId);
    int itemSz = sizeof(paramId);
    int type = 1;
    const char *param = (char *) &paramId;
    PGresult *charTuple = PQexecParams(conn,
        "DELETE FROM has_item WHERE char_id=$1 AND slot > 59", 1,
        NULL, &param, &itemSz, &type, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK) {
        PQclear(charTuple);
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't clear a player's old equipment list.");
        return;
    }

    PQclear(charTuple);

    //save equipment
    Item **equips = c->getEquipment();
    for (short i = 1; i < NUM_EQUIPS; i++) {
        if (!equips[i])
            continue;

        InsertEquip ie;
        ie.charId = charId;
        ie.itemId = equips[i]->getId();
        ie.slot = i + NUM_ITEMS;
        ie.qty = equips[i]->getQty();
        ie.dur = equips[i]->getDur();
        ie.mod = equips[i]->getMod();
        ie.identified = equips[i]->isIdentified();

        this->equips.push_back(std::move(ie));
    }

    //save lost items - first delete any temporaries
    charTuple = PQexecParams(conn,
        "DELETE FROM accident_item WHERE char_id=$1 AND temp=TRUE", 1,
        NULL, &param, &itemSz, &type, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK) {
        PQclear(charTuple);
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't clear a player's old temporary lost items.");
        return;
    }

    PQclear(charTuple);
    std::for_each(c->getLostItems().begin(), c->getLostItems().end(),
        [&](Item *i) {
            AccidentItem ai;
            ai.charId = charId;
            ai.itemId = i->getId();
            ai.dur = i->getDur();
            ai.qty = i->getQty();
            ai.mod = i->getMod();
            ai.temp = false;

            items.push_back(std::move(ai));
        });
    c->getLostItems().clear();

    //Save trade items as temps
    for (int j = 0; j < MAX_EXCHANGE; j++) {
        Item *i = (*c->getExchangeItems())[j];
        if (!i)
            continue;

        AccidentItem ai;
        ai.charId = charId;
        ai.itemId = i->getId();
        ai.dur = i->getDur();
        ai.qty = i->getQty();
        ai.mod = i->getMod();
        ai.temp = true;

        items.push_back(std::move(ai));
    }

    //Save skills
    Skill **skills = c->getSkills();
    for (unsigned short i = 0; i < NUM_SKILLS; i++) {
        if (!skills[i])
            continue;

        UpdateSkill us;
        us.charId = charId;
        us.level = skills[i]->getLevel();
        us.uses = skills[i]->getUseCount();
        us.cd = skills[i]->getCd();
        us.slot = i;

        this->skills.push_back(std::move(us));
    }

    Secret **secrets = c->getSecrets();
    for (unsigned short i = 0; i < NUM_SKILLS; i++) {
        if (!secrets[i])
            continue;

        UpdateSkill us;
        us.charId = charId;
        us.level = secrets[i]->getLevel();
        us.uses = secrets[i]->getUseCount();
        us.cd = secrets[i]->getCd();
        us.slot = i;

        this->secrets.push_back(std::move(us));
    }

    charTuple = PQexecParams(conn, "DELETE FROM has_effect WHERE char_id=$1", 1,
    NULL, &param, &itemSz, &type, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK) {
        PQclear(charTuple);
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't clear a player's old effects list.");
        return;
    }

    PQclear(charTuple);

    //TODO this and the corresponding in Character should be const
    StatusEffect **se = c->getEffects();
    for (int i = 0; i < StatusEffect::SE_KINDS; i++) {
        if (!se[i])
            continue;

        Effect e;
        e.charId = charId;
        e.buffId = se[i]->getId();
        e.duration = se[i]->getTickDuration();

        effects.push_back(std::move(e));
    }

    //Clear old trackers
    charTuple = PQexecParams(conn, "DELETE FROM trackers WHERE char_id=$1", 1,
    NULL, &param, &itemSz, &type, 0);

    //TODO make this STL
    for (Tracker *tracker : c->getTrackers()) {
        Tracker::TrackerIterator tie = tracker->end();
        for (Tracker::TrackerIterator ti = tracker->begin(); ti != tie; ++ti) {
            TrackerEntry te;
            std::pair<int, int> entry = *ti;
            te.charId = charId;
            te.mobId = entry.first;
            te.count = entry.second;
            te.questId = tracker->getQid();
            trackers.push_back(std::move(te));
        }
    }
}

void CharManager::ban(const char *name, int exp)
{
    std::string lname(name);
    lower(lname);
    banned[lname] = exp;
}

Character *CharManager::getCharacter(const char *lname, CharacterSession *who)
{
    if (banned[lname] > time(NULL))
        return 0;

    for (auto it = outgoing.begin(); it != outgoing.end(); it++) {
        std::string cname = (*it)->getName();
        lower(cname);
        if (cname == lname) {
            Character *res = *it;
            DisableUpdates<Character> du(res);
            res->setSession(who);
            res->reload();
            outgoing.erase(it);
            return res;
        }
    }

    Character *ret = loadCharacter(lname);
    if (!ret)
        return 0;
    ret->setSession(who);

    Database::getItems(ret);
    Database::getBuffs(ret);
    Database::getSkills(ret);

    return ret;
}

void CharManager::storageQtyChange(int charId, int itemId, short mod,
    int newQty)
{
    static int types[4] = { 1, 1, 1, 1 };
    static int sizes[4] = { sizeof(charId), sizeof(itemId), sizeof(mod),
        sizeof(newQty) };
    charId = htonl(charId);
    itemId = htonl(itemId);
    mod = htons(mod);
    newQty = htonl(newQty);
    const char *params[4] = { (const char *) &charId, (const char *) &itemId,
        (const char *) &mod, (const char *) &newQty };
    PGresult *su = PQexecParams(conn,
        "UPDATE storage SET qty=$4 WHERE char_id=$1 AND item_id=$2 AND mod=$3",
        4,
        NULL, params, sizes, types, 0);
    if (PQresultStatus(su) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't update the quantity of a character's stored items.");
    }
    PQclear(su);
}

void CharManager::storageAddItem(int charId, int itemId, short mod, int qty)
{
    static int types[4] = { 1, 1, 1, 1 };
    static int sizes[4] = { sizeof(charId), sizeof(itemId), sizeof(mod),
        sizeof(qty) };
    charId = htonl(charId);
    itemId = htonl(itemId);
    mod = htons(mod);
    qty = htonl(qty);
    const char *params[4] = { (const char *) &charId, (const char *) &itemId,
        (const char *) &mod, (const char *) &qty };
    PGresult *su =
        PQexecParams(conn,
            "INSERT INTO storage(char_id, item_id, qty, mod) VALUES ($1, $2, $4, $3)",
            4,
            NULL, params, sizes, types, 0);
    if (PQresultStatus(su) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't update the quantity of a character's stored items.");
    }
    PQclear(su);
}

void CharManager::storageRemoveItem(int charId, int itemId, short mod)
{
    static int types[3] = { 1, 1, 1 };
    static int sizes[3] = { sizeof(charId), sizeof(itemId), sizeof(mod) };
    charId = htonl(charId);
    itemId = htonl(itemId);
    mod = htons(mod);
    const char *params[3] = { (const char *) &charId, (const char *) &itemId,
        (const char *) &mod };
    PGresult *su = PQexecParams(conn,
        "DELETE FROM storage WHERE char_id=$1 AND item_id=$2 AND mod=$3", 3,
        NULL, params, sizes, types, 0);
    if (PQresultStatus(su) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't update the quantity of a character's stored items.");
    }
    PQclear(su);
}

void CharManager::addItem(int charId, int itemId, unsigned short slot,
    short mod, short qty, int dur, short idd)
{
    static int types[7] = { 1, 1, 1, 1, 1, 1, 1 };
    static int sizes[7] = { sizeof(charId), sizeof(itemId), sizeof(slot),
        sizeof(mod), sizeof(qty), sizeof(dur), sizeof(idd) };
    charId = htonl(charId);
    itemId = htonl(itemId);
    slot = htons(slot);
    mod = htons(mod);
    qty = htons(qty);
    dur = htonl(dur);
    idd = htons(idd);
    const char *params[7] = { (const char *) &charId, (const char *) &itemId,
        (const char *) &slot, (const char *) &mod, (const char *) &qty,
        (const char *) &dur, (const char *) &idd };
    PGresult *res =
        PQexecParams(conn,
            "INSERT INTO has_item(char_id, item_id, slot, mod, qty, dur, identified) VALUES ($1, $2, $3, $4, $5, $6, $7)",
            7,
            NULL, params, sizes, types, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't insert an item into a character's inventory.");
    }
    PQclear(res);
}

void CharManager::removeItem(int charId, unsigned short slot)
{
    static int types[2] = { 1, 1 };
    static int sizes[2] = { sizeof(charId), sizeof(slot) };
    charId = htonl(charId);
    slot = htons(slot);
    const char *params[2] = { (const char *) &charId, (const char *) &slot };
    PGresult *res = PQexecParams(conn,
        "DELETE FROM has_item WHERE char_id=$1 AND slot=$2", 2,
        NULL, params, sizes, types, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't remove an item into a character's inventory.");
    }
    PQclear(res);
}

void CharManager::updateItem(int charId, unsigned short slot, short mod,
    short qty, int dur, short idd)
{
    static int types[6] = { 1, 1, 1, 1, 1, 1 };
    static int sizes[6] = { sizeof(charId), sizeof(slot), sizeof(mod),
        sizeof(qty), sizeof(dur), sizeof(idd) };
    charId = htonl(charId);
    slot = htons(slot);
    mod = htons(mod);
    qty = htons(qty);
    dur = htonl(dur);
    idd = htons(idd);
    const char *params[6] = { (const char *) &charId, (const char *) &slot,
        (const char *) &mod, (const char *) &qty, (const char *) &dur,
        (const char *) &idd };
    PGresult *res =
        PQexecParams(conn,
            "UPDATE has_item SET mod=$3, qty=$4, dur=$5, identified=$6 WHERE char_id=$1 AND slot=$2",
            6,
            NULL, params, sizes, types, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't update an item in a character's inventory.");
    }
    PQclear(res);
}

void CharManager::addQuest(int charId, int qid, int qp, int timer)
{
    const char *params[4];
    charId = htonl(charId);
    qid = htonl(qid);
    qp = htonl(qp);
    timer = htonl(timer);
    params[0] = (char*) &charId;
    params[1] = (char*) &qid;
    params[2] = (char*) &qp;
    params[3] = (char*) &timer;
    static const int sizes[4] = { sizeof(charId), sizeof(qid), sizeof(qp),
        sizeof(timer) };
    static const int types[4] = { 1, 1, 1, 1 };

    PGresult *charTuple = PQexecParams(conn,
        "INSERT INTO quest_progress VALUES ($1, $2, $3, $4)", 4,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't insert a new quest.");
    }

    PQclear(charTuple);
}

void CharManager::updateQuest(int charId, int qid, int qp, int timer)
{
    const char *params[4];
    charId = htonl(charId);
    qid = htonl(qid);
    qp = htonl(qp);
    timer = htonl(timer);
    params[0] = (char*) &charId;
    params[1] = (char*) &qid;
    params[2] = (char*) &qp;
    params[3] = (char*) &timer;
    static const int sizes[4] = { sizeof(charId), sizeof(qid), sizeof(qp),
        sizeof(timer) };
    static const int types[4] = { 1, 1, 1, 1 };

    PGresult *charTuple =
        PQexecParams(conn,
            "UPDATE quest_progress SET quest_flags=$3, quest_timer=$4 WHERE char_id=$1 AND quest_id=$2",
            4,
            NULL, params, sizes, types, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK
        || strcmp(PQcmdTuples(charTuple), "1")) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a quest.");
    }

    PQclear(charTuple);
}

void CharManager::addMark(int charId, int markId, const char* textParam,
    int intParam, int timestamp)
{
    charId = htonl(charId);
    markId = htonl(markId);
    intParam = htonl(intParam);
    timestamp = htonl(timestamp);
    const char *params[5];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &markId;
    params[2] = textParam ? textParam : "";
    params[3] = (const char *) &intParam;
    params[4] = (const char *) &timestamp;

    static const int sizes[5] = { sizeof(charId), sizeof(markId), 0,
        sizeof(intParam), sizeof(timestamp) };
    static const int types[5] = { 1, 1, 0, 1, 1 };
    PGresult *charTuple =
        PQexecParams(conn,
            "INSERT INTO has_legend (char_id, mark_id, text_param, int_param, timestamp) VALUES ($1, $2, $3, $4, $5)",
            5,
            NULL, params, sizes, types, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't insert a new legend mark.");
    }

    PQclear(charTuple);
}

void CharManager::updateMark(int charId, int markId, const char* textParam,
    int intParam, int timestamp)
{
    charId = htonl(charId);
    markId = htonl(markId);
    intParam = htonl(intParam);
    timestamp = htonl(timestamp);
    const char *params[5];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &markId;
    params[2] = textParam;
    params[3] = (const char *) &intParam;
    params[4] = (const char *) &timestamp;
    static const int sizes[5] = { sizeof(charId), sizeof(markId), 0,
        sizeof(intParam), sizeof(timestamp) };
    static const int types[5] = { 1, 1, 0, 1, 1 };
    PGresult *charTuple =
        PQexecParams(conn,
            "UPDATE has_legend SET text_param=$3, int_param=$4, timestamp=$5 WHERE char_id=$1 AND mark_id=$2",
            5,
            NULL, params, sizes, types, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK
        || strcmp(PQcmdTuples(charTuple), "1")) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a legend mark.");
    }

    PQclear(charTuple);
}

void CharManager::deleteMark(int charId, int markId)
{
    charId = htonl(charId);
    markId = htonl(markId);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &markId;

    static const int sizes[2] = { sizeof(charId), sizeof(markId) };
    static const int types[2] = { 1, 1 };
    PGresult *charTuple = PQexecParams(conn,
        "DELETE FROM has_legend WHERE char_id=$1 AND mark_id=$2", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(charTuple) != PGRES_COMMAND_OK
        || strcmp(PQcmdTuples(charTuple), "1")) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't delete a legend mark.");
    }

    PQclear(charTuple);
}

void CharManager::addSkill(int charId, int skillId, unsigned short slot)
{
    charId = htonl(charId);
    skillId = htonl(skillId);
    slot = htons(slot);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &skillId;
    params[2] = (const char *) &slot;
    static const int sizes[3] =
        { sizeof(charId), sizeof(skillId), sizeof(slot) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "INSERT INTO has_skill VALUES ($1, $2, $3)", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't insert a new skill.");
    }

    PQclear(res);
}

void CharManager::removeSkill(int charId, unsigned short slot)
{
    charId = htonl(charId);
    slot = htons(slot);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &slot;
    static const int sizes[2] = { sizeof(charId), sizeof(slot) };
    static const int types[2] = { 1, 1 };

    PGresult *res = PQexecParams(conn,
        "DELETE FROM has_skill WHERE char_id=$1 AND slot=$2", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't delete a skill.");
    }

    PQclear(res);
}

void CharManager::moveSkill(int charId, unsigned short oldSlot,
    unsigned short newSlot)
{
    charId = htonl(charId);
    oldSlot = htons(oldSlot);
    newSlot = htons(newSlot);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &oldSlot;
    params[2] = (const char *) &newSlot;
    static const int sizes[3] = { sizeof(charId), sizeof(oldSlot),
        sizeof(newSlot) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE has_skill SET slot=$3 WHERE char_id=$1 AND slot=$2", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't move a skill!");
    }

    PQclear(res);
}

void CharManager::swapSkills(int charId, unsigned short slotOne,
    unsigned short slotTwo, int idOne, int idTwo)
{
    charId = htonl(charId);
    slotOne = htons(slotOne);
    slotTwo = htons(slotTwo);
    idOne = htonl(idOne);
    idTwo = htonl(idTwo);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &slotTwo;
    params[2] = (const char *) &idOne;
    static const int sizes[3] =
        { sizeof(charId), sizeof(slotTwo), sizeof(idOne) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE has_skill SET slot=$2 WHERE char_id=$1 AND skill_id=$3", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't swap skills.");
    }

    PQclear(res);

    params[1] = (const char *) &slotOne;
    params[2] = (const char *) &idTwo;

    res = PQexecParams(conn,
        "UPDATE has_skill SET slot=$2 WHERE char_id=$1 AND skill_id=$3", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't swap skills.");
    }

    PQclear(res);
}

void CharManager::addSecret(int charId, int skillId, unsigned short slot)
{
    charId = htonl(charId);
    skillId = htonl(skillId);
    slot = htons(slot);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &skillId;
    params[2] = (const char *) &slot;
    static const int sizes[3] =
        { sizeof(charId), sizeof(skillId), sizeof(slot) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "INSERT INTO has_secret VALUES ($1, $2, $3)", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't insert a new secret.");
    }

    PQclear(res);
}

void CharManager::removeSecret(int charId, unsigned short slot)
{
    charId = htonl(charId);
    slot = htons(slot);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &slot;
    static const int sizes[2] = { sizeof(charId), sizeof(slot) };
    static const int types[2] = { 1, 1 };

    PGresult *res = PQexecParams(conn,
        "DELETE FROM has_secret WHERE char_id=$1 AND slot=$2", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't delete a secret.");
    }

    PQclear(res);
}

void CharManager::moveSecret(int charId, unsigned short oldSlot,
    unsigned short newSlot)
{
    charId = htonl(charId);
    oldSlot = htons(oldSlot);
    newSlot = htons(newSlot);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &oldSlot;
    params[2] = (const char *) &newSlot;
    static const int sizes[3] = { sizeof(charId), sizeof(oldSlot),
        sizeof(newSlot) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE has_secret SET slot=$3 WHERE char_id=$1 AND slot=$2", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't move a secret.");
    }

    PQclear(res);
}

void CharManager::swapSecrets(int charId, unsigned short slotOne,
    unsigned short slotTwo, int idOne, int idTwo)
{
    charId = htonl(charId);
    slotOne = htons(slotOne);
    slotTwo = htons(slotTwo);
    idOne = htonl(idOne);
    idTwo = htonl(idTwo);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &slotTwo;
    params[2] = (const char *) &idOne;
    static const int sizes[3] =
        { sizeof(charId), sizeof(slotTwo), sizeof(idOne) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE has_secret SET slot=$2 WHERE char_id=$1 AND skill_id=$3", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't swap secrets.");
    }

    PQclear(res);

    params[1] = (const char *) &slotOne;
    params[2] = (const char *) &idTwo;

    res = PQexecParams(conn,
        "UPDATE has_secret SET slot=$2 WHERE char_id=$1 AND skill_id=$3", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't swap secrets.");
    }

    PQclear(res);
}

void CharManager::updatePath(int charId, unsigned short path,
    unsigned short pathmask)
{
    charId = htonl(charId);
    path = htons(path);
    pathmask = htons(pathmask);
    const char *params[3];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &path;
    params[2] = (const char *) &pathmask;
    static const int sizes[3] =
        { sizeof(charId), sizeof(path), sizeof(pathmask) };
    static const int types[3] = { 1, 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE characters SET path=$2, pathmask=$3 WHERE id=$1", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a player's path.");
    }

    PQclear(res);
}

void CharManager::updateHairstyle(int charId, unsigned short style)
{
    charId = htonl(charId);
    style = htons(style);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &style;
    static const int sizes[2] = { sizeof(charId), sizeof(style) };
    static const int types[2] = { 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE characters SET hairstyle=$2 WHERE id=$1", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a player's hairstyle.");
    }

    PQclear(res);
}

void CharManager::updateHaircolor(int charId, unsigned short color)
{
    charId = htonl(charId);
    color = htons(color);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &color;
    static const int sizes[2] = { sizeof(charId), sizeof(color) };
    static const int types[2] = { 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE characters SET haircolor=$2 WHERE id=$1", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a player's haircolor.");
    }

    PQclear(res);
}

void CharManager::updateNation(int charId, unsigned short nation)
{
    charId = htonl(charId);
    nation = htons(nation);
    const char *params[2];
    params[0] = (const char *) &charId;
    params[1] = (const char *) &nation;
    static const int sizes[2] = { sizeof(charId), sizeof(nation) };
    static const int types[2] = { 1, 1 };

    PGresult *res = PQexecParams(conn,
        "UPDATE characters SET nation=$2 WHERE id=$1", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't update a player's nation.");
    }

    PQclear(res);
}

void CharManager::lock()
{
    assert(LockSet::addLock(65536));
    cmlock.lock();
}

void CharManager::unlock()
{
    assert(LockSet::removeLock(65536));
    cmlock.unlock();
}

} /* namespace Database */
