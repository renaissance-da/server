/*
 * DataLoaders.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#define L_SHORT(x) ntohs(*((short*)(x)))
#define L_LONG(x) ntohl(*((uint32_t*)(x)))

#include "DataLoaders.h"
#include "SkillInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "crc.h"
#include "Map.h"
#include "DataService.h"
#include "Hash.h"
#include "Character.h"
#include "GameTime.h"
#include "BaseEquipment.h"
#include "Item.h"
#include "BaseConsumable.h"
#include "Field.h"
#include "SecretInfo.h"
#include "BaseEffect.h"
#include <algorithm>
#include <fstream>
#include "log4cplus/loggingmacros.h"
#include "Legend.h"
#include "MobInfo.h"
#include "config.h"

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

PGconn* conn;
log4cplus::Logger Database::log()
{
	return log4cplus::Logger::getInstance("renaissance.database");
}

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

void hostHashPw(const char *pw, uint32_t *pwRes, const char *salt)
{
    hashPw(pw, pwRes, salt);
    for (int i = 0; i < 8; i++) {
        pwRes[i] = htonl(pwRes[i]);
    }

    for (int i = 0; i < 1000; i++) {
        hashPw(pw, pwRes, (const char *) pwRes);
        for (int j = 0; j < 8; j++) {
            pwRes[j] = htonl(pwRes[j]);
        }
    }
}

bool Database::initDb()
{
    conn = PQconnectdb(config::db_conn_params);
    cm = new CharManager(config::db_conn_params);
    return PQstatus(conn) != CONNECTION_BAD;
}

void Database::closeDb()
{
    PQfinish(conn);
}

int Database::loadSkillDefs()
{
    //First, load status effects
    PGresult *status = PQexecParams(conn,
        "select buffid, kind, duration, icon, received, ended, param1, param2 "
            "FROM buffs", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);
    if (PQresultStatus(status) != PGRES_TUPLES_OK) {
        PQclear(status);
        return -1;
    }
    const int nBuffs = PQntuples(status);
    for (int i = 0; i < nBuffs; i++) {
        int id, kind, duration, param1, param2;
        short icon;
        const char *received, *ended;
        id = L_LONG(PQgetvalue(status, i, 0));
        kind = L_LONG(PQgetvalue(status, i, 1));
        duration = L_LONG(PQgetvalue(status, i, 2));
        icon = L_SHORT(PQgetvalue(status, i, 3));
        received = PQgetvalue(status, i, 4);
        ended = PQgetvalue(status, i, 5);
        param1 = L_LONG(PQgetvalue(status, i, 6));
        param2 = L_LONG(PQgetvalue(status, i, 7));

        new BaseEffect(id, kind, duration, icon, received, ended, param1,
            param2);
    }
    PQclear(status);

    PGresult *skills =
        PQexecParams(conn,
            "select s.skillid,s.name,s.max_warrior,s.max_rogue,s.max_wizard,s.max_priest,s.max_monk,s.max_master,"
                "s.anim,s.anim_time,s.sound,s.cd,s.icon,s.levelrate,s.effect,s.effect_self,"
                "s.strreq,s.dexreq,s.conreq,s.intreq,s.wisreq,s.levreq,s.pathreq,s.goldcost,s.item1,s.item1amt,"
                "s.item2,s.item2amt,s.item3,s.item3amt,e.sid,e.baselines,e.target,e.descfile,s.target,s.flags,e.elem,"
                "s.mpcost,s.item1mod,s.item2mod,s.item3mod,s.buff FROM skillinfo s LEFT JOIN secretinfo e ON s.skillid=e.sid",
            0,
            NULL,
            NULL,
            NULL,
            NULL, 1);

    if (PQresultStatus(skills) != PGRES_TUPLES_OK) {
        PQclear(skills);
        return -1;
    }

    int nSkills = PQntuples(skills);
    LOG4CPLUS_INFO(Database::log(),
        "Fetched " << nSkills << " skills from the database.");
    for (int i = 0; i < nSkills; i++) {
        SkillInfo *si;
        if (PQgetisnull(skills, i, 30)) {
            si = new SkillInfo;
            si->elem = -1;
        }
        else {
            SecretInfo *sc = new SecretInfo;
            sc->nlines = L_SHORT(PQgetvalue(skills, i, 31));
            sc->targets = L_SHORT(PQgetvalue(skills, i, 32));
            strcpy(sc->targetDesc, PQgetvalue(skills, i, 33));
            sc->elem = L_SHORT(PQgetvalue(skills, i, 36));
            si = sc;
        }
        strcpy(si->name, PQgetvalue(skills, i, 1));
        si->nameLen = strlen(si->name);
        for (int j = 0; j < 6; j++) {
            si->maxLevel[j] = ntohs(
                *((short *) (PQgetvalue(skills, i, j + 2))));
        }

        si->anim = ntohs(*((short *) (PQgetvalue(skills, i, 8))));
        si->animLen = ntohs(*((short *) (PQgetvalue(skills, i, 9))));
        si->sound = ntohs(*((short *) (PQgetvalue(skills, i, 10))));
        si->cd = ntohl(*((unsigned int *) PQgetvalue(skills, i, 11)));
        unsigned int id = ntohl(*((unsigned int *) PQgetvalue(skills, i, 0)));
        char icon = ntohs(*((short *) (PQgetvalue(skills, i, 12))));
        int levelrate = -1;
        unsigned short effect = 0, effectSelf = 0;
        if (!PQgetisnull(skills, i, 13))
            levelrate = L_LONG(PQgetvalue(skills, i, 13));
        if (!PQgetisnull(skills, i, 14))
            effect = L_SHORT(PQgetvalue(skills, i, 14));
        if (!PQgetisnull(skills, i, 15))
            effectSelf = L_SHORT(PQgetvalue(skills, i, 15));
        si->effect = effect;
        si->effectSelf = effectSelf;
        si->rate = levelrate;
        si->icon = icon;

        //costs
        short str, dex, con, int_, wis, lev, path, m1, m2, m3;
        int i1, a1, i2, a2, i3, a3, g, mp;
        BaseEffect *buff;
        str = dex = con = int_ = wis = lev = path = m1 = m2 = m3 = 0;
        i1 = a1 = i2 = a2 = i3 = a3 = g = 0;
        buff = 0;
        if (!PQgetisnull(skills, i, 16))
            str = L_SHORT(PQgetvalue(skills, i, 16));
        if (!PQgetisnull(skills, i, 17))
            dex = L_SHORT(PQgetvalue(skills, i, 17));
        if (!PQgetisnull(skills, i, 18))
            con = L_SHORT(PQgetvalue(skills, i, 18));
        if (!PQgetisnull(skills, i, 19))
            int_ = L_SHORT(PQgetvalue(skills, i, 19));
        if (!PQgetisnull(skills, i, 20))
            wis = L_SHORT(PQgetvalue(skills, i, 20));
        if (!PQgetisnull(skills, i, 21))
            lev = L_SHORT(PQgetvalue(skills, i, 21));
        if (!PQgetisnull(skills, i, 22))
            path = L_SHORT(PQgetvalue(skills, i, 22));
        if (!PQgetisnull(skills, i, 23))
            g = L_LONG(PQgetvalue(skills, i, 23));
        if (!PQgetisnull(skills, i, 24))
            i1 = L_LONG(PQgetvalue(skills, i, 24));
        if (!PQgetisnull(skills, i, 25))
            a1 = L_LONG(PQgetvalue(skills, i, 25));
        if (!PQgetisnull(skills, i, 26))
            i2 = L_LONG(PQgetvalue(skills, i, 26));
        if (!PQgetisnull(skills, i, 27))
            a2 = L_LONG(PQgetvalue(skills, i, 27));
        if (!PQgetisnull(skills, i, 28))
            i3 = L_LONG(PQgetvalue(skills, i, 28));
        if (!PQgetisnull(skills, i, 29))
            a3 = L_LONG(PQgetvalue(skills, i, 29));
        mp = L_LONG(PQgetvalue(skills, i, 37));
        if (!PQgetisnull(skills, i, 38))
            m1 = L_SHORT(PQgetvalue(skills, i, 38));
        if (!PQgetisnull(skills, i, 39))
            m2 = L_SHORT(PQgetvalue(skills, i, 39));
        if (!PQgetisnull(skills, i, 40))
            m3 = L_SHORT(PQgetvalue(skills, i, 40));
        if (!PQgetisnull(skills, i, 41)) {
            buff = BaseEffect::getById(L_LONG(PQgetvalue(skills, i, 41)));
            assert(buff);
        }

        //parameters
        TargetType tgt;
        int flags;

        tgt = (TargetType) L_SHORT(PQgetvalue(skills, i, 34));
        flags = L_LONG(PQgetvalue(skills, i, 35));

        si->str = str;
        si->dex = dex;
        si->con = con;
        si->int_ = int_;
        si->wis = wis;
        si->level = lev;
        si->path = path;
        si->gold = g;
        si->item1 = i1;
        si->item2 = i2;
        si->item3 = i3;
        si->item1Qty = a1;
        si->item2Qty = a2;
        si->item3Qty = a3;
        si->tgt = tgt;
        si->flags = flags;
        si->item1mod = m1;
        si->item2mod = m2;
        si->item3mod = m3;
        si->mp = mp;
        si->buff = buff;
        SkillInfo::addSkill(si, id);
    }

    PQclear(skills);

    PGresult *skReq = PQexecParams(conn,
        "SELECT skillid, reqid, levreq from skillreqskill", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);

    if (PQresultStatus(skReq) != PGRES_TUPLES_OK) {
        PQclear(skReq);
        return -1;
    }

    const int nReqs = PQntuples(skReq);
    for (int i = 0; i < nReqs; i++) {
        int id = L_LONG(PQgetvalue(skReq, i, 0));
        int req = L_LONG(PQgetvalue(skReq, i, 1));
        short lev = 0;
        if (!PQgetisnull(skReq, i, 2))
            lev = L_SHORT(PQgetvalue(skReq, i, 2));
        SkillInfo *si = SkillInfo::getById(id);
        if (!si || !SkillInfo::getById(req)) {
            LOG4CPLUS_ERROR(Database::log(),
                "Failed to load skill prerequisite for skill " << id << " because the skill wasn't found.");
        }
        else {
            SkillInfo::Req rq;
            rq.id = req;
            rq.lev = lev;
            si->reqs.push_back(rq);
        }
    }

    PQclear(skReq);

    return 0;
}

int Database::loadMaps(const char *mapDir)
{
    PGresult *fields = PQexecParams(conn, "SELECT id, field_name from field", 0,
    NULL,
    NULL,
    NULL,
    NULL, 1);

    if (PQresultStatus(fields) != PGRES_TUPLES_OK) {
        PQclear(fields);
        return -1;
    }

    const int nFields = PQntuples(fields);
    for (int i = 0; i < nFields; i++) {
        int id = L_LONG(PQgetvalue(fields, i, 0));
        const char *fname = PQgetvalue(fields, i, 1);
        Field *f = new Field(id);
        strcpy(f->name, fname);
        id = htonl(id);
        char *pid = (char *) (&id);
        int size = sizeof(id);
        int type = 1;
        PGresult *fieldWarps =
            PQexecParams(conn,
                "SELECT w.mapid, w.px, w.py, w.xdest, w.ydest, w.name "
                    "FROM field_dest w join field_dest_on o ON w.id=o.destid where o.fieldid=$1",
                1,
                NULL, &pid, &size, &type, 1);
        if (PQresultStatus(fieldWarps) != PGRES_TUPLES_OK) {
            PQclear(fieldWarps);
            continue;
        }
        const int nWarps = PQntuples(fieldWarps);
        for (int j = 0; j < nWarps; j++) {
            int mapid = L_LONG(PQgetvalue(fieldWarps, j, 0));
            int px = L_SHORT(PQgetvalue(fieldWarps, j, 1));
            int py = L_SHORT(PQgetvalue(fieldWarps, j, 2));
            int xdest = L_SHORT(PQgetvalue(fieldWarps, j, 3));
            int ydest = L_SHORT(PQgetvalue(fieldWarps, j, 4));
            const char *name = PQgetvalue(fieldWarps, j, 5);
            Field::FieldDest fd;
            fd.mapId = mapid;
            strcpy(fd.name, name);
            fd.x = px;
            fd.y = py;
            fd.xdest = xdest;
            fd.ydest = ydest;
            f->dests.push_back(fd);
        }

        PQclear(fieldWarps);
    }
    PQclear(fields);

    PGresult *maps = PQexecParams(conn,
        "select mapid,name,width,height,bgm,flags from map_db", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);
    PGresult *ports;

    if (PQresultStatus(maps) != PGRES_TUPLES_OK) {
        PQclear(maps);
        return -1;
    }

    int nMaps = PQntuples(maps);

    LOG4CPLUS_INFO(Database::log(),
        "Loaded " << nMaps << " maps from the database.");

    for (int i = 0; i < nMaps; i++) {
        unsigned int mapid = ntohl(
            *((unsigned int *) (PQgetvalue(maps, i, 0))));
        if (mapid > 0xFFFF) {
            LOG4CPLUS_ERROR(Database::log(),
                "Skipped map with id " << mapid << ", as max id is limited to 65535.");
            continue;
        }
        const char *name = PQgetvalue(maps, i, 1);
        unsigned short width = ntohs(*((short *) (PQgetvalue(maps, i, 2))));
        unsigned short height = ntohs(*((short *) (PQgetvalue(maps, i, 3))));
        unsigned short bgm = ntohs(*((short *) (PQgetvalue(maps, i, 4))));
        int flags = L_LONG(PQgetvalue(maps, i, 5));
        //need to open the map files to get the crc and wall map
        char wallName[PATH_MAX];
        snprintf(wallName, PATH_MAX, "%s/%u", mapDir, mapid);
        char fileName[PATH_MAX];
        snprintf(fileName, PATH_MAX, "%s/lod%u.map", mapDir, mapid);
        std::fstream walls, data;
        walls.open(wallName, std::ios::in | std::ios::binary);
        data.open(fileName, std::ios::in | std::ios::binary);
        if (!walls.is_open() || !data.is_open()) {
            LOG4CPLUS_ERROR(Database::log(),
                "Couldn't open map files for map " << mapid << ".");
            continue;
        }

        struct stat st;
        if (stat(fileName, &st) == -1) {
            LOG4CPLUS_ERROR(Database::log(),
                "Couldn't open map files for map " << mapid << ".");
            continue;
        }
        char *filedata = new char[st.st_size];
        //TODO error checking here
        data.read(filedata, st.st_size);
        unsigned short crc = getChecksum((unsigned char *) filedata,
            st.st_size);

        if (stat(wallName, &st) == -1) {
            LOG4CPLUS_ERROR(Database::log(),
                "Couldn't open map files for map " << mapid << ".");
            delete[] filedata;
            continue;
        }
        char *wallData = new char[st.st_size];
        walls.read(wallData, st.st_size);

        Map *m = new Map(name, width, height, bgm, crc,
            (unsigned char*) wallData, (unsigned short) mapid,
            (unsigned char *) filedata, flags);
        DataService::getService()->addMap(m, (unsigned short) mapid);
    }

    PQclear(maps);
    //load portals
    ports = PQexecParams(conn,
        "select mapid,x,y,destid,destx,desty,field_id from portal", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);

    if (PQresultStatus(ports) != PGRES_TUPLES_OK) {
        PQclear(ports);
        LOG4CPLUS_FATAL(Database::log(), "Failed to load portals.");
        return -1;
    }

    int nPorts = PQntuples(ports);
    for (int i = 0; i < nPorts; i++) {
        unsigned int msrc = ntohl(
            *((unsigned int *) (PQgetvalue(ports, i, 0))));
        unsigned int mdst = ntohl(
            *((unsigned int *) (PQgetvalue(ports, i, 3))));
        if (msrc > 0xFFFF || mdst > 0xFFFF) {
            LOG4CPLUS_ERROR(Database::log(),
                "Failed to load a portal between maps " << msrc << " and " << mdst << " (mapid too large).");
            continue;
        }
        unsigned short sx = ntohs(
            *((unsigned short *) (PQgetvalue(ports, i, 1))));
        unsigned short sy = ntohs(
            *((unsigned short *) (PQgetvalue(ports, i, 2))));
        unsigned short dx = ntohs(
            *((unsigned short *) (PQgetvalue(ports, i, 4))));
        unsigned short dy = ntohs(
            *((unsigned short *) (PQgetvalue(ports, i, 5))));
        Map *src = DataService::getService()->getMap(msrc);
        Map *dst = DataService::getService()->getMap(mdst);
        if (!src || !dst) {
            LOG4CPLUS_ERROR(Database::log(),
                "Couldn't load a portal because either the source " << msrc << " or dest " << mdst << " map was not loaded.");
        }
        Portal *p = src->addPortal(sx, sy, dx, dy, mdst);

        if (!PQgetisnull(ports, i, 6)) {
            int fid = L_LONG(PQgetvalue(ports, i, 6));
            p->makeField(fid);
        }
    }
    PQclear(ports);
    return 0;
}

Character *Database::loadCharacter(const char *charName)
{
    assert(strlen(charName) <= MAX_NAME_LEN);
    char name[MAX_NAME_LEN + 1];
    char *no = name;
    char *nameptr = name;
    char **pname = &nameptr;
    for (const char *cn = charName; *cn != '\0';) {
        *(no++) = tolower(*(cn++));
    }
    *no = '\0';
    PGresult *charTuple =
        PQexecParams(conn,
            "select id,mapid,x,y,hp,mp,maxHp,maxMp,strength,intelligence,wisdom,"
                "dexterity,constitution,hairstyle,haircolor,statPoints,gender,path,pathmask,"
                "level,ab,exp,ap,gold,priv,dead,labor,laborReset,nation,last_login,stored_gold,"
                "accident_gold,settings,guild_id,guild_rank,banned "
                "from characters where name=$1", 1,
            NULL, pname,
            NULL,
            NULL, 1);

    if (PQresultStatus(charTuple) != PGRES_TUPLES_OK) {
        PQclear(charTuple);
        LOG4CPLUS_WARN(Database::log(),
            "Failed to load character " << charName << ".");
        return 0;
    }
    else if (PQntuples(charTuple) != 1) {
        assert(PQntuples(charTuple) < 2);
        return 0;
    }

    int banned = L_LONG(PQgetvalue(charTuple, 0, 35));
    if (banned > time(NULL))
        return 0;

    int id = ntohl(*((int*) PQgetvalue(charTuple, 0, 0)));
    int map = ntohl(*((int*) PQgetvalue(charTuple, 0, 1)));
    short x = ntohs(*((short*) PQgetvalue(charTuple, 0, 2)));
    short y = ntohs(*((short*) PQgetvalue(charTuple, 0, 3)));
    int hp = ntohl(*((int*) PQgetvalue(charTuple, 0, 4)));
    int mp = ntohl(*((int*) PQgetvalue(charTuple, 0, 5)));
    int maxHp = ntohl(*((int*) PQgetvalue(charTuple, 0, 6)));
    int maxMp = ntohl(*((int*) PQgetvalue(charTuple, 0, 7)));
    short strength = ntohs(*((short*) PQgetvalue(charTuple, 0, 8)));
    short int_ = ntohs(*((short*) PQgetvalue(charTuple, 0, 9)));
    short wis = ntohs(*((short*) PQgetvalue(charTuple, 0, 10)));
    short dex = ntohs(*((short*) PQgetvalue(charTuple, 0, 11)));
    short con = ntohs(*((short*) PQgetvalue(charTuple, 0, 12)));
    short hairstyle = ntohs(*((short*) PQgetvalue(charTuple, 0, 13)));
    short haircolor = ntohs(*((short*) PQgetvalue(charTuple, 0, 14)));
    short statPoints = ntohs(*((short*) PQgetvalue(charTuple, 0, 15)));
    char gender = *PQgetvalue(charTuple, 0, 16);
    short path = ntohs(*((short*) PQgetvalue(charTuple, 0, 17)));
    short pathmask = ntohs(*((short*) PQgetvalue(charTuple, 0, 18)));
    short level = ntohs(*((short*) PQgetvalue(charTuple, 0, 19)));
    short ab = ntohs(*((short*) PQgetvalue(charTuple, 0, 20)));
    unsigned char *expb = (unsigned char *) PQgetvalue(charTuple, 0, 21);
    unsigned char *apb = (unsigned char *) PQgetvalue(charTuple, 0, 22);
    unsigned int gold = L_LONG(PQgetvalue(charTuple, 0, 23));
    unsigned short priv = L_SHORT(PQgetvalue(charTuple, 0, 24));
    unsigned short dead = L_SHORT(PQgetvalue(charTuple, 0, 25));
    unsigned short labor = L_SHORT(PQgetvalue(charTuple, 0, 26));
    unsigned int laborReset = L_LONG(PQgetvalue(charTuple, 0, 27));
    unsigned short nation = L_SHORT(PQgetvalue(charTuple, 0, 28));
    unsigned int lastLogin = L_LONG(PQgetvalue(charTuple, 0, 29));
    unsigned int storedGold = L_LONG(PQgetvalue(charTuple, 0, 30));
    unsigned int accidentGold = L_LONG(PQgetvalue(charTuple, 0, 31));
    int settings = L_LONG(PQgetvalue(charTuple, 0, 32));
    int gid = -1;
    short grank = -1;
    if (!PQgetisnull(charTuple, 0, 33))
        gid = L_LONG(PQgetvalue(charTuple, 0, 33));
    if (!PQgetisnull(charTuple, 0, 34))
        grank = L_SHORT(PQgetvalue(charTuple, 0, 34));
    long long exp = 0;
    long long ap = 0;
    for (int i = 0; i < 8; i++) {
        exp = (exp << 8) + expb[i];
        ap = (ap << 8) + apb[i];
    }

    gold += accidentGold;

    Map *m = DataService::getService()->getMap(map);
    if (!m) {
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't load character " << charName << " because they were " << " on map " << map << ", which isn't loaded.");
        PQclear(charTuple);
        return 0;
    }

    Character *res = new Character(id, charName, m, x, y, hp, mp, maxHp, maxMp,
        strength, int_, wis, dex, con, hairstyle, haircolor, statPoints, gender,
        path, pathmask, level, ab, exp, ap, gold, storedGold, priv, dead, labor,
        laborReset, nation, settings);
    // Set guild info
    if (gid >= 0) {
        Guild *g = Guild::getById(gid);
        if (!g)
            LOG4CPLUS_WARN(Database::log(),
                "Failed to find guild with ID " << gid << " while loading a character.");
        else
            Guild::getById(gid)->addMember(res, (Guild::Rank) grank, false);
    }
    PQclear(charTuple);

    if (!res)
        return 0;

    DisableUpdates<Character> du(res);

    id = htonl(id);
    const char *cid = (const char *) &id;
    const int sz = sizeof(id);
    const int type = 1;

    if (time(NULL) - lastLogin > 60 * 60 * 3 || m->isInstance())
        res->setNationCoords();
    else {
        //Load trackers
        PGresult *trackers = PQexecParams(conn,
            "SELECT quest_id, mob_id, qty FROM trackers "
                "WHERE char_id=$1 ORDER BY quest_id", 1,
            NULL, &cid, &sz, &type, 1);

        if (PQresultStatus(trackers) != PGRES_TUPLES_OK) {
            PQclear(trackers);
            LOG4CPLUS_ERROR(Database::log(),
                "Failed to query character " << ntohl(id) << "'s quest trackers.");
            return 0;
        }

        const int n = PQntuples(trackers);
        int i;
        for (i = 0; i < n; i++) {
            int qid = L_LONG(PQgetvalue(trackers, i, 0));
            if (qid)
                break;
            int mobId = L_LONG(PQgetvalue(trackers, i, 1));
            int qty = L_LONG(PQgetvalue(trackers, i, 2));

            res->addKill(mobId, qty);
        }

        for (; i < n; i++) {
            int qid = L_LONG(PQgetvalue(trackers, i, 0));
            assert(qid);
            std::vector<std::pair<int, int> > entries;
            while (i < n) {
                int nqid = L_LONG(PQgetvalue(trackers, i, 0));
                if (nqid != qid)
                    break;
                int mobId = L_LONG(PQgetvalue(trackers, i, 1));
                int qty = L_LONG(PQgetvalue(trackers, i, 2));
                entries.push_back( { mobId, qty });
                ++i;
            }

            int *ids = new int[entries.size()];
            int *qtys = new int[entries.size()];
            for (int j = 0; j < entries.size(); j++) {
                ids[j] = entries[j].first;
                qtys[j] = entries[j].second;
            }
            res->addTracker(qid, ids, entries.size(), qtys);
        }

        PQclear(trackers);
    }

    PGresult *legend = PQexecParams(conn,
        "SELECT l.mark_id,l.text_param,l.int_param,l.timestamp "
            "FROM has_legend l where l.char_id=$1 "
            "ORDER BY timestamp ASC", 1,
        NULL, &cid, &sz, &type, 1);

    if (PQresultStatus(legend) != PGRES_TUPLES_OK) {
        PQclear(legend);
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to query character " << ntohl(id) << "'s legend.");
        return 0;
    }

    const int n = PQntuples(legend);
    for (int i = 0; i < n; i++) {
        int mark_id = L_LONG(PQgetvalue(legend, i, 0));
        const char *textParam = PQgetvalue(legend, i, 1);
        int intParam = L_LONG(PQgetvalue(legend, i, 2));
        int timestamp = L_LONG(PQgetvalue(legend, i, 3));

        res->addLegendItem(mark_id, textParam, intParam, timestamp);
    }

    PQclear(legend);

    PGresult *quests =
        PQexecParams(conn,
            "SELECT quest_id, quest_flags, quest_timer FROM quest_progress WHERE char_id=$1",
            1,
            NULL, &cid, &sz, &type, 1);
    if (PQresultStatus(quests) != PGRES_TUPLES_OK) {
        PQclear(quests);
        LOG4CPLUS_WARN(Database::log(),
            "Failed to load quests for character " << charName << ".");
        return 0;
    }

    const int nq = PQntuples(quests);
    for (int i = 0; i < nq; i++) {
        int qid = L_LONG(PQgetvalue(quests, i, 0));
        int qp = L_LONG(PQgetvalue(quests, i, 1));
        int timer = L_LONG(PQgetvalue(quests, i, 2));
        res->setQuestProgress(qid, qp);
        res->setQuestTimer(qid, timer);
    }
    PQclear(quests);

    return res;

}

void Database::loadLegends()
{
    PGresult *legends = PQexecParams(conn,
        "SELECT id, prefix, text, param_fmt, icon, color from legend_mark", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);

    if (PQresultStatus(legends) != PGRES_TUPLES_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Couldn't load legend info from DB.");
        PQclear(legends);
        return;
    }

    const int nl = PQntuples(legends);
    for (int i = 0; i < nl; i++) {
        int id = L_LONG(PQgetvalue(legends, i, 0));
        const char *prefix = PQgetvalue(legends, i, 1);
        const char *text = PQgetvalue(legends, i, 2);
        short param_fmt = L_SHORT(PQgetvalue(legends, i, 3));
        short icon = L_SHORT(PQgetvalue(legends, i, 4));
        short color = L_SHORT(PQgetvalue(legends, i, 5));

        new Legend(id, prefix, text, (Legend::ParamFormat) param_fmt, icon,
            color);
    }

    PQclear(legends);
}

std::vector<std::pair<int, int> > Database::loadBanList()
{
    const int now = htonl(time(NULL));

    char *params[] = { (char *) &now };
    int types[] = { 1 };
    int sz[] = { sizeof(now) };

    PGresult *result = PQexecParams(conn,
        "SELECT addr, exp FROM ip_banlist WHERE exp > $1", 1,
        NULL, params, sz, types, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        PQclear(result);
        LOG4CPLUS_ERROR(Database::log(), "Failed to load the ban list.");
        return std::vector<std::pair<int, int> >();
    }

    std::vector<std::pair<int, int> > res;

    const int n = PQntuples(result);
    for (int i = 0; i < n; i++) {
        int addr, exp;
        addr = L_LONG(PQgetvalue(result, i, 0));
        exp = L_LONG(PQgetvalue(result, i, 1));
        res.push_back( { addr, exp });
    }

    return std::move(res);
}

void Database::banIp(int ipaddr, int expiry)
{
    ipaddr = htonl(ipaddr);
    expiry = htonl(expiry);
    char *params[] = { (char *) &ipaddr, (char *) &expiry };
    int types[] = { 1, 1 };
    int sz[] = { sizeof(ipaddr), sizeof(expiry) };

    PGresult *t = PQexecParams(conn, "DELETE FROM ip_banlist WHERE addr=$1", 1,
    NULL, params, sz, types, 1);
    if (PQresultStatus(t) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to clear an old ip banlist entry.");
    }
    PQclear(t);

    t = PQexecParams(conn, "INSERT INTO ip_banlist (addr, exp) VALUES ($1, $2)",
        2,
        NULL, params, sz, types, 1);
    if (PQresultStatus(t) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(), "Failed to add an ip banlist entry.");
    }
    PQclear(t);
}

void Database::setPassword(const char *name, const char *pw)
{
    unsigned int pass[8];
    char salt[32];
    initSalt(salt);

    hostHashPw(pw, pass, salt);

    size_t pwbytesLen, saltLen;
    unsigned char *pwbytes = PQescapeByteaConn(conn,
        (const unsigned char *) pass, 32, &pwbytesLen);
    unsigned char *saltbytes = PQescapeByteaConn(conn,
        (const unsigned char *) salt, 32, &saltLen);

    const char *params[3] = { name, (char *) pwbytes, (char *) saltbytes };
    PGresult *result = PQexecParams(conn,
        "UPDATE characters SET password=$2, salt=$3 WHERE name=$1", 3,
        NULL, params,
        NULL,
        NULL, 0);

    PQfreemem(pwbytes);
    PQfreemem(saltbytes);

    PQclear(result);
}

int Database::createGuild(const char *name)
{
    PGresult *result = PQexecParams(conn,
        "INSERT INTO guild (name) VALUES ($1)", 1,
        NULL, &name,
        NULL,
        NULL, 0);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        PQclear(result);
        return -1;
    }
    PQclear(result);

    //Return ID
    result = PQexecParams(conn, "SELECT id FROM guild WHERE name=$1", 1,
    NULL, &name,
    NULL,
    NULL, 1);

    if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) != 1) {
        LOG4CPLUS_WARN(Database::log(),
            "Created guild " << name << " but was unable to query its ID.");
        PQclear(result);
        return -1;
    }

    int id = L_LONG(PQgetvalue(result, 0, 0));
    PQclear(result);
    return id;
}

void Database::loadGuilds()
{
    PGresult *result = PQexecParams(conn, "SELECT name, id FROM guild", 0,
    NULL,
    NULL,
    NULL,
    NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        LOG4CPLUS_ERROR(Database::log(), "Failed to load existing guilds.");
        PQclear(result);
        return;
    }

    const int n = PQntuples(result);
    for (int i = 0; i < n; i++) {
        const char *name = PQgetvalue(result, i, 0);
        int gid = L_LONG(PQgetvalue(result, i, 1));
        Guild *g = new Guild(gid, name);

        // load member list
        int gidn = htonl(gid);
        const char *param = (const char *) &gidn;
        const int size = sizeof(int);
        const int type = 1;
        PGresult *members = PQexecParams(conn,
            "SELECT name, guild_rank FROM characters WHERE guild_id=$1", 1,
            NULL, &param, &size, &type, 1);
        if (PQresultStatus(members) != PGRES_TUPLES_OK) {
            LOG4CPLUS_ERROR(Database::log(),
                "Failed to load members of existing guild " << gid << ".");
        }
        else {
            const int m = PQntuples(members);
            for (int j = 0; j < m; j++) {
                const char *cname = PQgetvalue(members, j, 0);
                short rank = L_SHORT(PQgetvalue(members, j, 1));
                g->addToRoll(cname, (Guild::Rank) rank);
            }
        }
        PQclear(members);
    }

    PQclear(result);
}

void Database::addToGuild(int charId, int guildId, short rank)
{
    charId = htonl(charId);
    guildId = htonl(guildId);
    rank = htons(rank);
    const char *params[3] = { (const char *) &charId, (const char *) &guildId,
        (const char *) &rank };
    const int sizes[3] = { sizeof(int), sizeof(int), sizeof(short) };
    const int types[3] = { 1, 1, 1 };

    PGresult *result = PQexecParams(conn,
        "UPDATE characters SET guild_id=$2, guild_rank=$3 "
            "WHERE id=$1", 3,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to add character " << ntohl(charId) << " to guild " << ntohl(guildId) << ".");
    }

    PQclear(result);
}

void Database::updateGuildRank(const char *name, short rank)
{
    rank = htons(rank);
    const char *params[2] = { name, (const char *) &rank };
    const int sizes[2] = { 0, sizeof(short) };
    const int types[2] = { 0, 1 };

    PGresult *result = PQexecParams(conn, "UPDATE characters SET guild_rank=$2 "
        "WHERE name=$1", 2,
    NULL, params, sizes, types, 0);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to update character " << name << "'s guild rank.");
    }

    PQclear(result);
}

void Database::removeFromGuild(const char *charName)
{
    const char *params[1] = { charName };

    PGresult *result = PQexecParams(conn, "UPDATE characters SET guild_id=0 "
        "WHERE name=$1", 1,
    NULL, params,
    NULL,
    NULL, 0);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to remove character " << charName << " from a guild.");
    }

    PQclear(result);
}
void Database::deleteGuild(int guildId)
{
    guildId = htonl(guildId);
    const char *params[1] = { (const char *) &guildId };
    const int sizes[1] = { sizeof(int) };
    const int types[1] = { 1 };

    PGresult *result = PQexecParams(conn, "DELETE FROM guild WHERE id=$1", 1,
    NULL, params, sizes, types, 0);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to delete guild " << ntohl(guildId) << ".");
    }

    PQclear(result);
}

bool Database::makeCharacter(const char *name, const char *pw)
{
    assert(strlen(name) <= MAX_NAME_LEN);
    unsigned int pass[8];

    char salt[32];
    initSalt(salt);

    hostHashPw(pw, pass, salt);
    char lname[MAX_NAME_LEN + 1];
    char *lno = lname;
    for (const char *c = name; *c != '\0';) {
        *(lno++) = tolower(*(c++));
    }
    *lno = '\0';

    size_t pwbytesLen, saltLen;
    unsigned char *pwbytes = PQescapeByteaConn(conn,
        (const unsigned char *) pass, 32, &pwbytesLen);
    unsigned char *saltbytes = PQescapeByteaConn(conn,
        (const unsigned char *) salt, 32, &saltLen);

    const char *params[3] = { lname, (char *) pwbytes, (char *) saltbytes };
    PGresult *result = PQexecParams(conn,
        "INSERT INTO characters VALUES ($1, $2, $3)", 3,
        NULL, params,
        NULL,
        NULL, 0);

    PQfreemem(pwbytes);
    PQfreemem(saltbytes);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        PQclear(result);
        return false;
    }

    PQclear(result);

    PQexecParams(conn,
        "INSERT INTO has_skill VALUES ((SELECT id FROM characters WHERE name=$1),1,0)",
        1,
        NULL, params,
        NULL,
        NULL, 0);

    char btime[30];
    gameDateseason(btime, 30);
    snprintf(btime, 11, "%ld", time(NULL));
    params[0] = btime;
    params[1] = lname;

    PGresult *lm =
        PQexecParams(conn,
            "INSERT INTO has_legend VALUES ((SELECT id FROM characters WHERE name=$2),1,' ',0,$1)",
            2,
            NULL, params,
            NULL,
            NULL, 0);

    PQclear(lm);

    return true;
}

void Database::delLegend(int id, const char *prefix)
{
    id = htonl(id);
    const char *params[2] = { (const char *) &id, prefix };
    int sizes[2] = { sizeof(int), 0 };
    int types[2] = { 1, 0 };

    PGresult *clearLegend = PQexecParams(conn,
        "DELETE FROM legend WHERE char_id=$1 AND prefix=$2", 2,
        NULL, params, sizes, types, 0);

    if (PQresultStatus(clearLegend) != PGRES_COMMAND_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to clear legend mark " << prefix << " from character " << ntohl(id) << ".");
    }

    PQclear(clearLegend);
}

bool Database::setAttributes(const char *name, unsigned char hair,
    unsigned char haircolor, char gender)
{

    PGresult *result = PQexecParams(conn,
        "SELECT id FROM characters WHERE name=$1", 1,
        NULL, &name,
        NULL,
        NULL, 1);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        PQclear(result);
        return false;
    }
    if (PQntuples(result) != 1) {
        PQclear(result);
        return false;
    }
    int id_n = *((int*) PQgetvalue(result, 0, 0));

    char upd[200];
    snprintf(upd, 200, "UPDATE characters "
        "SET attribute_flag=1, hairstyle=%hhu, haircolor=%hhu, gender='%c' "
        "WHERE id=%d AND attribute_flag=0", hair, haircolor, gender,
        ntohl(id_n));

    result = PQexec(conn, upd);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        PQclear(result);
        return false;
    }
    if (strcmp(PQcmdTuples(result), "1")) {
        PQclear(result);
        return false;
    }

    PQclear(result);

    //Give starter gear
    unsigned int itemid = htonl(gender == 'm' ? 0 : 1);
    const char *params[2] = { (const char *) &id_n, (const char *) &itemid };
    int types[2] = { 1, 1 };
    int sizes[2] = { sizeof(int), sizeof(unsigned int) };
    result = PQexecParams(conn, "INSERT INTO has_item VALUES "
        "($1,$2,0,NULL,1000)", 2,
    NULL, params, sizes, types, 0);
    PQclear(result);

    return true;
}

bool Database::changePassword(const char *name, const char *oldPw,
    const char *newPw)
{
    if (testPassword(name, oldPw))
        return false;

    setPassword(name, newPw);
    return true;

}

int Database::testPassword(const char *name, const char *pw)
{
    if (strlen(name) > MAX_NAME_LEN)
        return E_INVALID;

    PGresult *pwTuple = PQexecParams(conn,
        "SELECT password,salt FROM characters WHERE name=$1", 1,
        NULL, &name,
        NULL,
        NULL, 1);

    if (PQresultStatus(pwTuple) != PGRES_TUPLES_OK) {
        PQclear(pwTuple);
        LOG4CPLUS_WARN(Database::log(),
            "Query failed while testing password for " << "character " << name << ".");
        return E_UNSPECIFIED;
    }
    if (PQntuples(pwTuple) != 1) {
        assert(PQntuples(pwTuple) < 2);
        PQclear(pwTuple);
        return E_NOEXIST;
    }

    //Read password
    unsigned int *actPw = (unsigned int *) PQgetvalue(pwTuple, 0, 0);
    const char *salt = PQgetvalue(pwTuple, 0, 1);
    unsigned int givenPw[8];

    hostHashPw(pw, givenPw, salt);
    int res = 0;
    for (int i = 0; i < 8; i++) {
        if (actPw[i] != givenPw[i]) {
            res = E_WRONGPW;
            break;
        }
    }

    PQclear(pwTuple);
    return res;
}

void Database::getSkills(Character *c)
{

    int id = htonl(c->getId()), length = sizeof(int), type = 1;
    const char *id_p = (const char *) (&id);

    PGresult *skillTuples =
        PQexecParams(conn,
            "SELECT skill_id,slot,level,uses,cd FROM has_skill WHERE char_id=$1::integer",
            1,
            NULL, &id_p, &length, &type, 1);
    if (PQresultStatus(skillTuples) != PGRES_TUPLES_OK) {
        PQclear(skillTuples);
        return;
    }

    const int n = PQntuples(skillTuples);
    for (int i = 0; i < n; i++) {
        int sk_id = ntohl(*((int*) PQgetvalue(skillTuples, i, 0)));
        short slot = ntohs(*((short*) PQgetvalue(skillTuples, i, 1)));
        short level = ntohs(*((short*) PQgetvalue(skillTuples, i, 2)));
        unsigned int uses = L_LONG(PQgetvalue(skillTuples, i, 3));
        unsigned int cd = L_LONG(PQgetvalue(skillTuples, i, 4));
        c->addSkill(sk_id, level, slot, uses, cd);
    }

    PQclear(skillTuples);

    PGresult *spellTuples =
        PQexecParams(conn,
            "SELECT skill_id,slot,level,uses,cd FROM has_secret WHERE char_id=$1::integer",
            1,
            NULL, &id_p, &length, &type, 1);
    if (PQresultStatus(spellTuples) != PGRES_TUPLES_OK) {
        PQclear(spellTuples);
        return;
    }

    const int m = PQntuples(spellTuples);
    for (int i = 0; i < m; i++) {
        int sk_id = ntohl(*((int*) PQgetvalue(spellTuples, i, 0)));
        short slot = ntohs(*((short*) PQgetvalue(spellTuples, i, 1)));
        short level = ntohs(*((short*) PQgetvalue(spellTuples, i, 2)));
        unsigned int uses = L_LONG(PQgetvalue(spellTuples, i, 3));
        unsigned int cd = L_LONG(PQgetvalue(spellTuples, i, 4));
        c->addSecret(sk_id, level, slot, uses, cd);
    }

    PQclear(spellTuples);
}

void Database::getItems(Character *c)
{
    DisableUpdates<Character> du(c);

    unsigned int id = htonl(c->getId());
    int type = 1;
    int size = sizeof(id);
    const char *param = (const char *) &id;
    PGresult *items =
        PQexecParams(conn,
            "SELECT item_id, slot, qty, dur, mod FROM has_item WHERE char_id=$1 ORDER BY slot ASC",
            1,
            NULL, &param, &size, &type, 1);

    if (PQresultStatus(items) != PGRES_TUPLES_OK) {
        PQclear(items);
        return;
    }

    const int n = PQntuples(items);
    int i;
    for (i = 0; i < n; i++) {
        unsigned short slot = L_SHORT(PQgetvalue(items, i, 1));
        unsigned int id = L_LONG(PQgetvalue(items, i, 0)), dur = 0;
        unsigned short qty = 0, mod = 0;
        if (!PQgetisnull(items, i, 2))
            qty = L_SHORT(PQgetvalue(items, i, 2));
        if (!PQgetisnull(items, i, 3))
            dur = L_LONG(PQgetvalue(items, i, 3));
        if (!PQgetisnull(items, i, 4))
            mod = L_SHORT(PQgetvalue(items, i, 4));

        Item *item = new Item(id, qty, dur);
        if (mod) {
            ((Equipment *) item)->setMod(mod);
        }
        c->putItem(item, slot);
    }

    PQclear(items);

    //load storage
    PGresult *storage = PQexecParams(conn,
        "SELECT item_id, qty, mod from storage where char_id=$1", 1,
        NULL, &param, &size, &type, 1);

    if (PQresultStatus(storage) != PGRES_TUPLES_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Failed to load storage for character " << c->getName() << ".");
        PQclear(storage);
        return;
    }

    for (int i = 0; i < PQntuples(storage); i++) {
        int itemId = L_LONG(PQgetvalue(storage, i, 0));
        int qty = L_LONG(PQgetvalue(storage, i, 1));
        short mod = L_SHORT(PQgetvalue(storage, i, 2));

        BaseItem *bi = BaseItem::getById(itemId);
        if (!bi) {
            LOG4CPLUS_WARN(Database::log(),
                "Character " << c->getName() << " is storing an item " << "with ID " << itemId << ", which doesn't exist.");
            continue;
        }
        c->setStored(itemId, qty, mod);
    }
    PQclear(storage);

    //Check for lost items
    PGresult *overflow =
        PQexecParams(conn,
            "SELECT item_id, qty, dur, mod, id FROM accident_item WHERE char_id=$1 ORDER BY id ASC",
            1,
            NULL, &param, &size, &type, 1);

    if (PQresultStatus(overflow) != PGRES_TUPLES_OK) {
        LOG4CPLUS_WARN(Database::log(),
            "Couldn't load accident storage for player " << c->getName() << ".");
        PQclear(overflow);
        return;
    }

    const int m = PQntuples(overflow);
    long long lastSuccess = 0;
    Item *lost = 0;
    for (i = 0; i < m; i++) {
        unsigned int id = L_LONG(PQgetvalue(overflow, i, 0));
        unsigned short qty = L_SHORT(PQgetvalue(overflow, i, 1));
        unsigned int dur = L_LONG(PQgetvalue(overflow, i, 2));
        unsigned short mod = L_SHORT(PQgetvalue(overflow, i, 3));
        lastSuccess = *((long long*) PQgetvalue(overflow, i, 4));

        lost = new Item(id, qty, dur);
        if (mod && lost->getType() == BaseItem::EQUIP)
            ((Equipment*) lost)->setMod(mod);
        if (!c->getItem(lost))
            break;

        lost = 0;
    }
    PQclear(overflow);

    char *params[5] = { (char *) &id, (char *) &lastSuccess };
    int sizes[5] = { sizeof(id), sizeof(lastSuccess) };
    int types[5] = { 1, 1, 1, 1, 1 };

    overflow = PQexecParams(conn,
        "DELETE FROM accident_item WHERE char_id=$1 AND id <= $2", 2,
        NULL, params, sizes, types, 1);

    if (PQresultStatus(overflow) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Failed to delete some items from the overflow list\n");
        LOG4CPLUS_WARN(Database::log(),
            "Failed to delete some items from character " << ntohl(id) << "'s overflow list.");
        PQclear(overflow);
        return;
    }

    PQclear(overflow);

    if (!lost)
        return;

    //Add last lost item back to db (in case the quantity was changed)
    int itemId, dur;
    short mod, qty;
    itemId = htonl(lost->getId());
    dur = htonl(lost->getDur());
    mod = htons(lost->getMod());
    qty = htons(lost->getQty());
    params[1] = (char *) &itemId;
    params[2] = (char *) &qty;
    params[3] = (char *) &dur;
    params[4] = (char *) &mod;
    sizes[1] = sizeof(itemId);
    sizes[2] = sizeof(qty);
    sizes[3] = sizeof(dur);
    sizes[4] = sizeof(mod);
    overflow =
        PQexecParams(conn,
            "INSERT INTO accident_item(char_id, item_id, qty, dur, mod) VALUES ($1, $2, $3, $4, $5)",
            5,
            NULL, params, sizes, types, 1);

    if (PQresultStatus(overflow) != PGRES_COMMAND_OK)
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to re-add an overflow item " << ntohl(itemId) << "(x" << ntohl(qty) << ", mod: " << ntohs(mod) << ") for character " << ntohl(id) << ".");

    PQclear(overflow);

    //mark all temporaries as permanent since they've become lost at this point
    overflow = PQexecParams(conn,
        "UPDATE accident_item SET temp=FALSE where char_id=$1", 1,
        NULL, params, sizes, types, 1);

    if (PQresultStatus(overflow) != PGRES_COMMAND_OK)
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to mark temporary items as permanent " << "on character " << ntohl(id) << ".");

    PQclear(overflow);

    delete lost;
}

void Database::getBuffs(Character *c)
{
    unsigned int id = htonl(c->getId());
    int type = 1;
    int size = sizeof(id);
    const char *param = (const char *) &id;

    PGresult *effects = PQexecParams(conn,
        "SELECT buff_id, duration from has_effect WHERE char_id=$1", 1,
        NULL, &param, &size, &type, 1);
    if (PQresultStatus(effects) != PGRES_TUPLES_OK) {
        PQclear(effects);
        LOG4CPLUS_WARN(Database::log(),
            "Failed to query character " << ntohl(id) << "'s buffs.");
    }

    const int ne = PQntuples(effects);
    for (int i = 0; i < ne; i++) {
        int buffid = L_LONG(PQgetvalue(effects, i, 0));
        int duration = L_LONG(PQgetvalue(effects, i, 1));
        c->addStatusEffect(buffid, duration);
    }
    PQclear(effects);
}

void Database::loadMobs()
{
    using std::vector;
    using std::pair;
    using mob::MobInfo;

    PGresult *mobs = PQexecParams(conn,
        "SELECT id, level, hplo, hpmax, name, minDmg, maxDmg, exp, apr, "
            "mode, submode, gold_min, gold_max, mr, dex, size, regen, atk, "
            "defense, ac, power FROM mob", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);

    if (PQresultStatus(mobs) != PGRES_TUPLES_OK) {
        PQclear(mobs);
        LOG4CPLUS_ERROR(Database::log(),
            "Couldn't fetch mobs from the database.");
        return;
    }

    const int n = PQntuples(mobs);
    for (int i = 0; i < n; i++) {
        int id = L_LONG(PQgetvalue(mobs, i, 0));
        char *id_c = PQgetvalue(mobs, i, 0);
        short level = L_SHORT(PQgetvalue(mobs, i, 1));
        int hplo = L_LONG(PQgetvalue(mobs, i, 2));
        int hpmax = L_LONG(PQgetvalue(mobs, i, 3));
        const char *name = PQgetvalue(mobs, i, 4);
        int mindmg = L_LONG(PQgetvalue(mobs, i, 5));
        int maxdmg = L_LONG(PQgetvalue(mobs, i, 6));
        int exp = L_LONG(PQgetvalue(mobs, i, 7));
        short apr = L_SHORT(PQgetvalue(mobs, i, 8));
        short mode = L_SHORT(PQgetvalue(mobs, i, 9));
        short submode = L_SHORT(PQgetvalue(mobs, i, 10));
        int minGold = L_LONG(PQgetvalue(mobs, i, 11));
        int maxGold = L_LONG(PQgetvalue(mobs, i, 12));
        short mr = L_SHORT(PQgetvalue(mobs, i, 13));
        short dex = L_SHORT(PQgetvalue(mobs, i, 14));
        char size = *PQgetvalue(mobs, i, 15);
        short regen = L_SHORT(PQgetvalue(mobs, i, 16));
        short atk = L_SHORT(PQgetvalue(mobs, i, 17));
        short defense = L_SHORT(PQgetvalue(mobs, i, 18));
        short ac = L_SHORT(PQgetvalue(mobs, i, 19));
        short power = L_SHORT(PQgetvalue(mobs, i, 20));

        char **param = &id_c;
        int sz = 4;
        int type = 1;
        PGresult *drops = PQexecParams(conn,
            "SELECT item_id,rate,mod FROM drops WHERE mob_id = $1", 1,
            NULL, param, &sz, &type, 1);
        if (PQresultStatus(drops) != PGRES_TUPLES_OK) {
            LOG4CPLUS_WARN(Database::log(),
                "Mob info identified by ID " << id << " not created " << "because drops couldn't be queried.");
            PQclear(drops);
            continue;
        }

        const int nDrops = PQntuples(drops);
        vector<MobInfo::Drop> dropList;
        for (int j = 0; j < nDrops; j++) {
            unsigned int itemId = L_LONG(PQgetvalue(drops, j, 0));
            unsigned short rate = L_SHORT(PQgetvalue(drops, j, 1));
            short mod = L_SHORT(PQgetvalue(drops, j, 2));
            BaseItem *bi = BaseItem::getById(itemId);
            if (!bi)
                continue;
            MobInfo::Drop d;
            d.bi = bi;
            d.rate = rate;
            d.mod = mod;
            dropList.push_back(d);
        }

        PQclear(drops);

        PGresult *skills = PQexecParams(conn,
            "SELECT skill_id, rate FROM mob_skill where mob_id=$1", 1,
            NULL, param, &sz, &type, 1);
        if (PQresultStatus(skills) != PGRES_TUPLES_OK) {
            LOG4CPLUS_WARN(Database::log(),
                "Mob info identified by ID " << id << " not created because skills couldn't be queried.");
            PQclear(skills);
            continue;
        }

        vector<pair<int, short> > skillList;
        for (int i = 0; i < PQntuples(skills); i++) {
            int skid = L_LONG(PQgetvalue(skills, i, 0));
            short rate = L_LONG(PQgetvalue(skills, i, 1));
            if (SkillInfo::getById(skid))
                skillList.push_back(pair<int, short>(skid, rate));
        }

        PQclear(skills);

        //add mob info
        new MobInfo(id, level, hplo, name, mindmg, maxdmg, exp, apr, mode, atk,
            defense, ac, power, std::move(dropList), minGold, maxGold, mr, dex,
            hpmax, size, regen, submode, std::move(skillList));
    }

    PQclear(mobs);
}

void Database::loadSpawners()
{
    PGresult *spawners = PQexecParams(conn,
        "SELECT mob_id, map_id, qty, frequency FROM spawner", 0,
        NULL,
        NULL,
        NULL,
        NULL, 1);

    if (PQresultStatus(spawners) != PGRES_TUPLES_OK) {
        PQclear(spawners);
        LOG4CPLUS_FATAL(Database::log(), "Couldn't query spawners.");
        return;
    }

    const int n = PQntuples(spawners);
    {
        LOG4CPLUS_INFO(Database::log(),
            "Fetched " << n << " spawners from the database.");
    }

    for (int i = 0; i < n; i++) {
        int mapid = L_LONG(PQgetvalue(spawners, i, 1));
        short qty = L_SHORT(PQgetvalue(spawners, i, 2));
        int mobid = L_LONG(PQgetvalue(spawners, i, 0));
        int frequency = L_LONG(PQgetvalue(spawners, i, 3));

        Map *m = DataService::getService()->getMap(mapid);
        if (!m) {
            LOG4CPLUS_ERROR(Database::log(),
                "Spawner assigned to map " << mapid << " not created " << "because the map is not loaded.");
        }

        Spawner *s = new Spawner(mobid, m, qty, frequency);
        m->addSpawner(s);
    }
    PQclear(spawners);
}

void loadEquip(unsigned int id)
{
    const char *param = (char *) &id;
    int sz[1] = { 4 };
    int type[1] = { 1 };
    PGresult *eqp =
        PQexecParams(conn,
            "SELECT i.ground_apr,i.weight,i.name,i.id_name,e.slot,e.dur,e.gender,e.equip_apr,e.underwear,"
                "e.level,e.path,e.wMin,e.wMax,e.ac,e.hp,e.mp,e.hit,e.dmg,e.mr,e.strength,e.con,e.dex,"
                "e.intelligence,e.wisdom,e.ele,i.value,e.regen,e.improvable,e.flags,e.lmin,e.lmax,i.flags "
                "FROM item i JOIN equip e on i.id=e.item_id WHERE i.id=$1", 1,
            NULL, &param, sz, type, 1);

    id = ntohl(id);

    if (PQresultStatus(eqp) != PGRES_TUPLES_OK) {
        PQclear(eqp);
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to load equipment item with ID " << id << ".");
        return;
    }

    short apr = L_SHORT(PQgetvalue(eqp, 0, 0));
    short weight = L_SHORT(PQgetvalue(eqp, 0, 1));
    char *name = PQgetvalue(eqp, 0, 2);
    char *id_name = PQgetvalue(eqp, 0, 3);
    short slot = L_SHORT(PQgetvalue(eqp, 0, 4));
    int dur = L_LONG(PQgetvalue(eqp, 0, 5));
    char *gender = PQgetvalue(eqp, 0, 6);
    short eqpApr = L_SHORT(PQgetvalue(eqp, 0, 7));
    short extraApr = L_SHORT(PQgetvalue(eqp, 0, 8));
    short level = L_SHORT(PQgetvalue(eqp, 0, 9));
    short path = L_SHORT(PQgetvalue(eqp, 0, 10));
    int wMin = L_LONG(PQgetvalue(eqp, 0, 11));
    int wMax = L_LONG(PQgetvalue(eqp, 0, 12));
    short ac = L_SHORT(PQgetvalue(eqp, 0, 13));
    int hp = L_LONG(PQgetvalue(eqp, 0, 14));
    int mp = L_LONG(PQgetvalue(eqp, 0, 15));
    short hit = L_SHORT(PQgetvalue(eqp, 0, 16));
    short dmg = L_SHORT(PQgetvalue(eqp, 0, 17));
    short mr = L_SHORT(PQgetvalue(eqp, 0, 18));
    short str = L_SHORT(PQgetvalue(eqp, 0, 19));
    short con = L_SHORT(PQgetvalue(eqp, 0, 20));
    short dex = L_SHORT(PQgetvalue(eqp, 0, 21));
    short int_ = L_SHORT(PQgetvalue(eqp, 0, 22));
    short wis = L_SHORT(PQgetvalue(eqp, 0, 23));
    short ele = L_SHORT(PQgetvalue(eqp, 0, 24));
    int value = L_LONG(PQgetvalue(eqp, 0, 25));
    short regen = L_SHORT(PQgetvalue(eqp, 0, 26));
    bool improvable = L_SHORT(PQgetvalue(eqp, 0, 27));
    int flags = L_LONG(PQgetvalue(eqp, 0, 28));
    int lMin = L_LONG(PQgetvalue(eqp, 0, 29));
    int lMax = L_LONG(PQgetvalue(eqp, 0, 30));
    unsigned int bflags = L_LONG(PQgetvalue(eqp, 0, 31));

    new BaseEquipment(name, apr, weight, value, id, slot, gender ? *gender : 0,
        level, eqpApr, extraApr, dur, path, wMin, wMax, ac, hp, mp, hit, dmg,
        mr, str, con, dex, int_, wis, ele, regen, flags, improvable, lMin, lMax,
        bflags);

    PQclear(eqp);

}

void loadItem(unsigned int id)
{
    const char *param = (char *) &id;
    int sz[1] = { 4 };
    int type[1] = { 1 };
    PGresult *item =
        PQexecParams(conn,
            "SELECT ground_apr,weight,name,value,id_name,max_stack,flags FROM item WHERE id=$1",
            1,
            NULL, &param, sz, type, 1);
    id = htonl(id);

    if (PQresultStatus(item) != PGRES_TUPLES_OK) {
        PQclear(item);
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to load item with " << "ID " << id << ".");
        return;
    }

    unsigned short apr = L_SHORT(PQgetvalue(item, 0, 0));
    unsigned short weight = L_SHORT(PQgetvalue(item, 0, 1));
    char *name = PQgetvalue(item, 0, 2);
    unsigned int value = L_LONG(PQgetvalue(item, 0, 3));
    char *id_name = PQgetvalue(item, 0, 4);
    unsigned short maxStack;
    if (PQgetisnull(item, 0, 5))
        maxStack = 0;
    else
        maxStack = L_SHORT(PQgetvalue(item, 0, 5));
    unsigned int flags = L_LONG(PQgetvalue(item, 0, 6));

    new BaseItem(name, apr, weight, value, maxStack, id, false, BaseItem::PLAIN,
        flags);

    PQclear(item);
}

void loadConsume(unsigned int id)
{
    const char *param = (char *) &id;
    int sz[1] = { 4 };
    int type[1] = { 1 };
    PGresult *item =
        PQexecParams(conn,
            "SELECT i.ground_apr,i.weight,i.name,i.value,i.id_name,i.max_stack,c.type,c.param,"
                "i.flags "
                "FROM item i JOIN consumable c ON i.id=c.item_id WHERE id=$1",
            1,
            NULL, &param, sz, type, 1);
    id = htonl(id);

    if (PQresultStatus(item) != PGRES_TUPLES_OK) {
        PQclear(item);
        LOG4CPLUS_ERROR(Database::log(),
            "Failed to load consumable with ID " << id << ".");
        return;
    }

    unsigned short apr = L_SHORT(PQgetvalue(item, 0, 0));
    unsigned short weight = L_SHORT(PQgetvalue(item, 0, 1));
    char *name = PQgetvalue(item, 0, 2);
    unsigned int value = L_LONG(PQgetvalue(item, 0, 3));
    char *id_name = PQgetvalue(item, 0, 4);
    unsigned short maxStack;
    if (PQgetisnull(item, 0, 5))
        maxStack = 0;
    else
        maxStack = L_SHORT(PQgetvalue(item, 0, 5));
    short ctype = L_SHORT(PQgetvalue(item, 0, 6));
    int cparam = L_LONG(PQgetvalue(item, 0, 7));
    unsigned int flags = L_LONG(PQgetvalue(item, 0, 8));

    //new BaseItem(name, apr, weight, maxStack, id, BaseItem::PLAIN);
    new BaseConsumable(name, apr, weight, value, maxStack, id, ctype, cparam,
        flags);

    PQclear(item);
}

void Database::loadItems()
{
    PGresult *items =
        PQexecParams(conn,
            "SELECT DISTINCT i.id, e.item_id, c.item_id from item i LEFT JOIN equip e ON (i.id=e.item_id) "
                "LEFT JOIN consumable c ON (i.id=c.item_id)", 0,
            NULL,
            NULL,
            NULL,
            NULL, 1);

    if (PQresultStatus(items) != PGRES_TUPLES_OK) {
        PQclear(items);
        LOG4CPLUS_FATAL(Database::log(), "Couldn't query items.");
        return;
    }

    const int n = PQntuples(items);
    {
        LOG4CPLUS_INFO(Database::log(),
            "Loaded " << n << " items from the database.");
    }

    for (int i = 0; i < n; i++) {
        if (!PQgetisnull(items, i, 1))
            loadEquip(*((int*) PQgetvalue(items, i, 1)));
        else if (!PQgetisnull(items, i, 2))
            loadConsume(*((int*) PQgetvalue(items, i, 2)));
        else
            loadItem(*((int*) PQgetvalue(items, i, 0)));
    }

    PQclear(items);
}
