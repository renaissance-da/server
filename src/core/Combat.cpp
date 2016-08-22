/*
 * Combat.cpp
 *
 *  Created on: 2013-02-23
 *      Author: per
 */

#include "Combat.h"
#include "defines.h"
#include "srv_proto.h"
#include "random_engines.h"
#include "DataService.h"
#include "element.h"
#include <assert.h>
#include "Trap.h"
#include "StatusTrap.h"
#include "npc_bindings.h"
#include <math.h>
#include "GameTime.h"
#include <algorithm>
#include "Paths.h"
#include "Consumable.h"
#include "Mob.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

void radSearch(std::vector<MapPoint> &area, const unsigned short &x,
    const unsigned short &y, unsigned short r)
{
    for (unsigned short dx = 0; dx < r; dx++) {
        //top
        area.push_back(MapPoint(x + dx, y + dx - r, 0));
        //right
        area.push_back(MapPoint(x + r - dx, y + dx, 0));
        //bottom
        area.push_back(MapPoint(x - dx, y + r - dx, 0));
        //left
        area.push_back(MapPoint(x + dx - r, y - dx, 0));
    }
}

/**
 * \brief Acquire all valid targets of the skill given
 *
 * Acquire all valid targets of the skill given. Valid targets depend on
 * the range and type of the skill used.
 * \param[in] a The entity who is using the skill.
 * \param[in] sk The skill being used.
 * \param[in] tgt The target of the skill. Ignored if the skill doesn't use
 * a specific target.
 * \param[out] targets A collection of valid targets.
 */
void Combat::getTargets(Entity *a, Skill *sk, Entity *tgt,
    std::vector<Entity *> &targets)
{
    unsigned short x, y, tx, ty;
    short dx, dy, rad = 0;
    MapPoint *mp;
    Map *m;
    std::vector<MapPoint> area;

    x = a->getX();
    y = a->getY();
    m = a->getMap();

    if (tgt) {
        tx = tgt->getX();
        ty = tgt->getY();
    }
    else {
        //Arbitrary
        tx = ty = 0x8000;
    }

    switch (a->getDir()) {
    case UP:
        dx = 0;
        dy = -1;
        break;
    case DOWN:
        dx = 0;
        dy = 1;
        break;
    case LEFT:
        dx = -1;
        dy = 0;
        break;
    default:
        dx = 1;
        dy = 0;
        break;
    }

    switch (sk->getTargetType()) {
    case TARGET_TARGET:
        if (tgt)
            targets.push_back(tgt);
        break;
    case TARGET_FRONT:
        mp = new MapPoint(x + dx, y + dy, 0);
        tgt = m->getEnt(mp);
        delete mp;
        if (tgt)
            targets.push_back(tgt);
        break;
    case TARGET_SELF_LARGE:
        //add all dist 3 targets
        radSearch(area, x, y, 3);
        /* no break */
    case TARGET_SELF_MED:
        radSearch(area, x, y, 2);
        /* no break */
    case TARGET_SELF_SMALL:
        area.push_back(MapPoint(x, y, 0));
        /* no break */
    case TARGET_CIRCLE:
        radSearch(area, x, y, 1);
        break;
    case TARGET_LARGE:
        radSearch(area, tx, ty, 3);
        /* no break */
    case TARGET_MED:
        radSearch(area, tx, ty, 2);
        /* no break */
    case TARGET_SMALL:
        if (tgt) {
            radSearch(area, tx, ty, 1);
            area.push_back(MapPoint(tx, ty, 0));
        }
        else {
            area.clear();
        }
        break;
    case TARGET_ALL:
        m->getNearbyEntities(a, targets);
        break;
    case TARGET_GROUP:
        assert(a->getType() == Entity::E_CHARACTER);
        if (!((Character *) a)->getGroup())
            return;
        std::for_each(((Character *) a)->getGroup()->begin(),
            ((Character *) a)->getGroup()->end(), [&](Character *c) {
                if (c != a)
                targets.push_back(c);
            });
        break;
    case TARGET_LINE_LARGE:
        area.push_back(MapPoint(x, y, 0));
        rad = 3;
        x += dx;
        y += dy;
        /* no break */
    case TARGET_LINE_SMALL:
        rad += 3;
        for (short p = 1; p <= rad; p++) {
            area.push_back(MapPoint(x + p * dx, y + p * dy, 0));
        }
        break;
    case TARGET_GROUP_VISIBLE:
        assert(a->getType() == Entity::E_CHARACTER);
        if (!((Character *) a)->getGroup())
            return;
        std::for_each(((Character *) a)->getGroup()->begin(),
            ((Character *) a)->getGroup()->end(), [&](Character *c) {
                if (c != a) {
                    if (c->dist(a) <= NEARBY)
                    targets.push_back(c);
                }
            });
        break;
    case TARGET_SELF:
        targets.push_back(a);
        break;
    case TARGET_FIRST_LINE:
        for (short p = 1; p < 4; p++) {
            mp = new MapPoint(x + p * dx, y + p * dy, 0);
            tgt = m->getEnt(mp);
            delete mp;
            if (tgt) {
                targets.push_back(tgt);
                break;
            }
        }
        break;
    default:
        break;
    }

    m->getEnts(area, targets, a);
    if (sk->getFlags() & PLAYER_ONLY) {
        for (auto it = targets.begin(); it != targets.end();) {
            if ((*it)->getType() != Entity::E_CHARACTER)
                it = targets.erase(it);
            else
                ++it;
        }
    }
    else if (sk->getFlags() & MOB_ONLY) {
        for (auto it = targets.begin(); it != targets.end();) {
            if ((*it)->getType() != Entity::E_MOB)
                it = targets.erase(it);
            else
                ++it;
        }
    }
}

bool Combat::specialEffect(Character *user, Skill *sk,
    std::vector<Entity *> &targets)
{
    std::uniform_int_distribution<int> percent_dist(0, 99);
  
    Entity *target = targets.size() ? targets[0] : 0;
    
    char buffer[1024];
    short dx, dy, prefDir;
    std::vector<Entity *> auxillary;
    switch (user->getDir()) {
    case UP:
        dx = 0;
        dy = -1;
        prefDir = DOWN;
        break;
    case DOWN:
        dx = 0;
        dy = 1;
        prefDir = UP;
        break;
    case LEFT:
        dx = -1;
        dy = 0;
        prefDir = RIGHT;
        break;
    case RIGHT:
        dx = 1;
        dy = 0;
        prefDir = LEFT;
        break;
    default:
        dx = dy = 0;
        prefDir = UP; //arbitrary
        break;
    }

    switch (sk->getId()) {
    Skill *chargeStrike;
    char weaponString[100], armorString[100];
    char *buff_append;
    Item *itm;
    short maxDist;
case SK_LOOK:
    if (target) {
        if (target->getType() == Entity::E_CHARACTER) {
            if (((Character *) target)->getEquipment()[Equipment::WEAPON])
                snprintf(weaponString, 100, "w:Weapon :%s",
                    ((Character *) target)->getEquipment()[Equipment::WEAPON]->getName());
            else
                snprintf(weaponString, 100, "0:none  :NoName");
            if (((Character *) target)->getEquipment()[Equipment::ARMOR])
                snprintf(armorString, 100, "a:Armor :%s",
                    ((Character *) target)->getEquipment()[Equipment::ARMOR]->getName());
            else
                snprintf(armorString, 100, "0:none  :NoName");

            buff_append = buffer
                + snprintf(buffer, 1024, "WORLD %s\n\n%s\n %s\n %s\n\n",
                    user->getMap()->getName(), target->getName().c_str(),
                    weaponString, armorString);
        }
        else {
            buff_append = buffer
                + snprintf(buffer, 1024, "WORLD %s\n\n%s\n\n",
                    user->getMap()->getName(), target->getName().c_str());
        }
    }
    else
        buff_append = buffer
            + snprintf(buffer, 1024, "WORLD %s\n\n", user->getMap()->getName());

    {
        std::vector<GroundItem *> items;
        user->getMap()->getItems(user->getX() + dx, user->getY() + dy, items);
        for (GroundItem *e : items) {
            if (buff_append >= buffer + 1024)
                break;
            buff_append += snprintf(buff_append, 1024 + buffer - buff_append,
                "%s\n", e->getName().c_str());
        }
    }

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_MARTIAL_AWARENESS:
case SK_COMBAT_SENSES:
case SK_ECHO_SENSE:
case SK_STUDY_CREATURE:
    if (!target || target->getType() != Entity::E_MOB)
        return target; //Will improve on anything
    if (sk->getLevel() < 30)
        snprintf(buffer, 1024,
            "   ***Sense Monster***   \nName: %14s Exp: %u\n",
            target->getName().c_str(), ((Mob*) target)->getExp());
    else if (sk->getLevel() < 60)
        snprintf(buffer, 1024,
            "   ***Sense Monster***   \nName: %14s Exp: %u\nLevel: %u\nHp: %u\n",
            target->getName().c_str(), ((Mob*) target)->getExp(),
            target->getLevel(), target->getHp());
    else if (sk->getLevel() < 90)
        snprintf(buffer, 1024,
            "   ***Sense Monster***\nName: %14s Exp: %u\nLevel: %u\nHp: %u\n"
                "Attack Nature: %s\n", target->getName().c_str(),
            ((Mob*) target)->getExp(), target->getLevel(), target->getHp(),
            getEleName(target->getAtkEle()));
    else
        snprintf(buffer, 1024,
            "   ***Sense Monster***\nName: %14s Exp: %u\nLevel: %u\nHp: %u\n"
                "Attack Nature: %s\nDefense Nature: %s\n",
            target->getName().c_str(), ((Mob*) target)->getExp(),
            target->getLevel(), target->getHp(),
            getEleName(target->getAtkEle()), getEleName(target->getDefEle()));
    Server::sendMessage(user->getSession(), buffer, 8);
    return true;
case SK_PERISH_LORE:
    itm = user->getInventory()[0];
    if (!itm)
        return false;
    if (itm->getDeathAction() == Item::PERISH)
        snprintf(buffer, 256, "%s\nPerish YES", itm->getName());
    else
        snprintf(buffer, 256, "%s\nPerish NO", itm->getName());
    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_MELEE_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::EQUIP
        || ((Equipment*) itm)->getSlot() != Equipment::WEAPON
        || (((Equipment*) itm)->getFlags() & 2))
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024,
            "%s\nWeight: %14hu\nValue: %14u\nArmor: %3hd Hit: %3hd Dmg: %3hd\n"
                "Durability: %d / %d\nDamage:    S:%14d-%d\n           L:%14d-%d\n%s Level %6hu",
            itm->getName(), itm->getWeight(), itm->getValue(),
            ((Equipment*) itm)->getStats()->getAc(),
            ((Equipment*) itm)->getStats()->getHit(),
            ((Equipment*) itm)->getStats()->getDmg(), itm->getDur(),
            itm->getMaxDur(), ((Equipment*) itm)->getStats()->getAtkMin(),
            ((Equipment*) itm)->getStats()->getAtkMax(),
            ((Equipment*) itm)->getStats()->getAtkMinL(),
            ((Equipment*) itm)->getStats()->getAtkMaxL(),
            paths[((Equipment*) itm)->getPath()].name,
            ((Equipment*) itm)->levelReq());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_ARMOR_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::EQUIP
        || ((Equipment*) itm)->getSlot() != Equipment::ARMOR)
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024,
            "%s\nWeight: %14hu\nValue: %14u\nArmor: %3hd Hit: %3hd Dmg: %3hd\n"
                "Durability: %d / %d\n%s Level %6hu", itm->getName(),
            itm->getWeight(), itm->getValue(),
            ((Equipment*) itm)->getStats()->getAc(),
            ((Equipment*) itm)->getStats()->getHit(),
            ((Equipment*) itm)->getStats()->getDmg(), itm->getDur(),
            itm->getMaxDur(), paths[((Equipment*) itm)->getPath()].name,
            ((Equipment*) itm)->levelReq());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_MISSILE_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::EQUIP
        || ((Equipment*) itm)->getSlot() != Equipment::WEAPON
        || !(((Equipment*) itm)->getFlags() & 2))
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024,
            "%s\nWeight: %14hu\nValue: %14u\nArmor: %3hd Hit: %3hd Dmg: %3hd\n"
                "Durability: %d / %d\nDamage:    S:%14d-%d\n           L:%14d-%d\n%s Level %6hu",
            itm->getName(), itm->getWeight(), itm->getValue(),
            ((Equipment*) itm)->getStats()->getAc(),
            ((Equipment*) itm)->getStats()->getHit(),
            ((Equipment*) itm)->getStats()->getDmg(), itm->getDur(),
            itm->getMaxDur(), ((Equipment*) itm)->getStats()->getAtkMin(),
            ((Equipment*) itm)->getStats()->getAtkMax(),
            ((Equipment*) itm)->getStats()->getAtkMinL(),
            ((Equipment*) itm)->getStats()->getAtkMaxL(),
            paths[((Equipment*) itm)->getPath()].name,
            ((Equipment*) itm)->levelReq());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_EVALUATE_ITEM:
case SK_WISE_TOUCH:
case SK_ANALYZE:
    itm = user->getInventory()[0];
    if (!itm)
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024, "%s\nWeight: %14hu\nValue: %14u", itm->getName(),
            itm->getWeight(), itm->getValue());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_APPRAISE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::PLAIN)
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024, "%s\nWeight: %14hu\nValue: %14u", itm->getName(),
            itm->getWeight(), itm->getValue());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_POTION_LORE:
case SK_HERBAL_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::CONSUME)
        snprintf(buffer, 1024, "You can't do it.");
    else {
        switch (((Consumable*) itm)->getType()) {
        case BaseConsumable::HP_POTION:
            if (((Consumable*) itm)->getParam() > 1000000)
                snprintf(buffer, 1024, "%s\nGreatly restores vitality.",
                    itm->getName());
            else if (((Consumable*) itm)->getParam() < -1000000)
                snprintf(buffer, 1024, "%s\nGreatly damages vitality.",
                    itm->getName());
            else
                snprintf(buffer, 1024, "%s\n%s %d vitality.", itm->getName(),
                    (((Consumable*) itm)->getParam() >= 0) ?
                        "Restores" : "Drains up to",
                    ((Consumable*) itm)->getParam() >= 0 ?
                        ((Consumable*) itm)->getParam() :
                        -((Consumable*) itm)->getParam());
            break;
        case BaseConsumable::MP_POTION:
            if (((Consumable*) itm)->getParam() > 1000000)
                snprintf(buffer, 1024, "%s\nGreatly restores will.",
                    itm->getName());
            else if (((Consumable*) itm)->getParam() < -1000000)
                snprintf(buffer, 1024, "%s\nGreatly damages will.",
                    itm->getName());
            else
                snprintf(buffer, 1024, "%s\n%s %d will.", itm->getName(),
                    (((Consumable*) itm)->getParam() >= 0) ?
                        "Restores" : "Drains",
                    ((Consumable*) itm)->getParam() >= 0 ?
                        ((Consumable*) itm)->getParam() :
                        -((Consumable*) itm)->getParam());
            break;
        case BaseConsumable::REVIVE:
            snprintf(buffer, 1024, "%s\nRevives its target.", itm->getName());
            break;
        default:
            snprintf(buffer, 1024, "You can't do it.");
            break;
        }
    }

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_FOOD_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::CONSUME
        || ((Consumable*) itm)->getType() != BaseConsumable::FOOD)
        snprintf(buffer, 1024, "You can't do it.");
    else
        snprintf(buffer, 1024, "%s\n%s %d vitality.", itm->getName(),
            ((Consumable*) itm)->getParam() < 0 ? "Drains" : "Restores",
            ((Consumable*) itm)->getParam() < 0 ?
                -((Consumable*) itm)->getParam() :
                ((Consumable*) itm)->getParam());

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_MAGIC_LORE:
    itm = user->getInventory()[0];
    if (!itm || itm->getType() != BaseItem::EQUIP)
        snprintf(buffer, 1024, "You can't do it.");
    else {
        buff_append = buffer + snprintf(buffer, 1024, "%s\n", itm->getName());
        if (((Equipment*) itm)->getStats()->getHp())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Vitality increase:%10d\n",
                ((Equipment*) itm)->getStats()->getHp());
        if (((Equipment*) itm)->getStats()->getMp())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Will increase:%14d\n",
                ((Equipment*) itm)->getStats()->getMp());
        if (((Equipment*) itm)->getStats()->getStr())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Strength:%19hd\n", ((Equipment*) itm)->getStats()->getStr());
        if (((Equipment*) itm)->getStats()->getInt())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Intelligence:%15hd\n",
                ((Equipment*) itm)->getStats()->getInt());
        if (((Equipment*) itm)->getStats()->getWis())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Wisdom:%21d\n", ((Equipment*) itm)->getStats()->getWis());
        if (((Equipment*) itm)->getStats()->getCon())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Constitution:%15d\n",
                ((Equipment*) itm)->getStats()->getCon());
        if (((Equipment*) itm)->getStats()->getDex())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Grace:%22d\n", ((Equipment*) itm)->getStats()->getDex());
        if (((Equipment*) itm)->getStats()->getHit())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Hit increase:%15d\n",
                ((Equipment*) itm)->getStats()->getHit());
        if (((Equipment*) itm)->getStats()->getDmg())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Damage increase:%12d\n",
                ((Equipment*) itm)->getStats()->getDmg());
        if (((Equipment*) itm)->getStats()->getMr())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Protection increase:%8d\n",
                ((Equipment*) itm)->getStats()->getMr());
        if (((Equipment*) itm)->getStats()->getRegen())
            buff_append += snprintf(buff_append, 1024 - (buff_append - buffer),
                "Healing increase:%11d\n",
                ((Equipment*) itm)->getStats()->getRegen());
    }

    Server::sendMessage(user->getSession(), buffer, 8);
    return false;
case SK_NEEDLE_TRAP:
case SK_STILETTO_TRAP:
case SK_COILED_BOLT_TRAP:
case SK_BOLT_TRAP:
case SK_MAIDEN_TRAP:
    user->getMap()->addTrigger(
        new Trap(user->getX(), user->getY(), user->getMap(), user,
            skillDmg(user, 0, sk)));
    return true;
case SK_POISON_TRAP:
case SK_POISON_SNARE:
case SK_GREATER_POISON_SNARE:
case SK_GREAT_POISON_SNARE:
case SK_BLIND_TRAP:
case SK_BLIND_SNARE:
case SK_GREATER_BLIND_SNARE:
case SK_GREAT_BLIND_SNARE:
case SK_SLEEP_TRAP:
case SK_SLEEP_SNARE:
case SK_GREATER_SLEEP_SNARE:
case SK_GREAT_SLEEP_SNARE:
    user->getMap()->addTrigger(
        new StatusTrap(user->getX(), user->getY(), user->getMap(),
            sk->getBase()->buff->id, user->getType()));
    return true;
case SK_NIS:
    gameDatetime(buffer, 128);
    Server::sendMessage(user->getSession(), buffer, 3);
    return false;
case SK_SHADOW_FIGURE:
    //TODO add damage part
case SK_AMBUSH:
    if (!target)
        return false;

    //Check for walls
    for (int i = 1; i < 4; i++) {
        if (target->getX() == user->getX() + dx * i
            && target->getY() == user->getY() + dy * i)
            break;
        if (user->getMap()->isWall(user->getX() + i * dx, user->getY() + i * dy,
            user->moveType()))
            return false;
    }

    if (skillRate(user, target, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    //Try the opposite side of the mob
    user->setDir(prefDir);
    if (user->getMap()->mapJump(user, target->getX() + dx, target->getY() + dy))
        return true;
    //What about CW
    user->setDir((prefDir + 1) % 4);
    if (user->getMap()->mapJump(user, target->getX() - dy, target->getY() + dx))
        return true;
    //or CCW
    user->setDir((prefDir + 3) % 4);
    if (user->getMap()->mapJump(user, target->getX() + dy, target->getY() - dx))
        return true;
    //or even 180
    user->setDir((prefDir + 2) % 4);
    if (user->getMap()->mapJump(user, target->getX() - dx, target->getY() - dy))
        return true;
    Server::sendMessage(user->getSession(), "You can't do it.");
    return true;
case SK_CHARGE:
    if (skillRate(user, 0, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    for (short i = 1; i < 5; i++) {
        if (!user->getMap()->mapJump(user, user->getX() + dx,
            user->getY() + dy))
            break;
    }

    chargeStrike = new Skill(SK_CHARGE_STRIKE);
    useSkill(user, chargeStrike);
    delete chargeStrike;

    return true;
case SK_BULLRUSH:
    if (skillRate(user, 0, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    for (maxDist = 1; maxDist < 3; maxDist++) {
        if (user->getMap()->isWall(user->getX() + dx * maxDist,
            user->getY() + dy * maxDist, user->moveType()))
            break;
    }

    if (maxDist > 1)
        user->getMap()->mapJump(user, user->getX() + dx * (maxDist - 1),
            user->getY() + dy * (maxDist - 1));

    if (!target)
        return true;

    if (target->getX() == user->getX() + dx
        && target->getY() == user->getY() + dy) {
        chargeStrike = new Skill(SK_BULLRUSH_STRIKE);
        user->getMap()->mapJump(target, target->getX() + dx,
            target->getY() + dy);
        useSkill(user, chargeStrike, target);
        delete chargeStrike;
    }

    return true;
case SK_BACKSLIDE:
    if (skillRate(user, 0, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    if (target) {
        target->addStatusEffect(305);
        target->playEffect(57, 100);
        target->makeSound(8);
    }

    for (maxDist = 0; maxDist < 3; maxDist++) {
        if (!user->getMap()->mapJump(user, user->getX() - dx,
            user->getY() - dy))
            break;
    }
    return true;
case SK_TWISTER_STRIKE:
    if (!target)
        return false;

    if (skillRate(user, 0, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    //TODO want to use the targeting mode to select targets rather than constructing
    // a skill with the appropriate targets
    chargeStrike = new Skill(10606);
    getTargets(user, chargeStrike, 0, auxillary);
    delete chargeStrike;
    chargeStrike = new Skill(71, Master);

    user->setDir(prefDir);
    if (!user->getMap()->mapJump(user, user->getX() + (dx << 1),
        user->getY() + (dy << 1))) {
        Server::sendMessage(user->getSession(), "You can't do it.");
        user->setDir((prefDir + 2) & 3);
        delete chargeStrike;
        return false;
    }

    target->getMap()->mapJump(target, target->getX() - dx, target->getY() - dy);

    for (Entity *e : auxillary) {
        if (e != target) {
            chargeStrike->levelUp();
            e->playEffect(285, 100);
        }
    }

    useSkill(user, chargeStrike, target);
    delete chargeStrike;
    return true;
case SK_CAIRDE_GLAOCH:
    for (auto e : targets) {
        if (e == user)
            continue;
        if (e->getType() == Entity::E_CHARACTER)
            script::recall((Character *) e, user->getMapId(), user->getX(),
                user->getY());
    }
    return false;
case SK_FAS_SPIORAD:
    if (skillRate(user, 0, sk) <= percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed");
        return true;
    }

    if ((signed) user->getHp() <= user->getStats()->getMp() * 2 / 5) {
        //user->incMp((user->getHp() - 100) * 5 / 2);
        user->addMp(user->getStats()->getMp());
        user->damage(user, user->getHp() - 100);
    }
    else {
        user->damage(user, user->getStats()->getMp() * 2 / 5);
        user->addMp(user->getStats()->getMp());
    }
    user->playEffect(sk->getBase()->effect, 0x64);

    return true;
case SK_BEAG_BREISLEICH:
    if (!target || target->getLevel() > 33) {
        Server::sendMessage(user->getSession(), "Failed.");
        return false;
    }
    /*no break*/
case SK_BREISLEICH:
    if (!target || target->getLevel() > 66) {
        Server::sendMessage(user->getSession(), "Failed.");
        return false;
    }
    /*no break*/
case SK_MOR_BREISLEICH:
case SK_AMNESIA:
    if (!target || skillRate(user, target, sk) < percent_dist(generator())) {
        Server::sendMessage(user->getSession(), "Failed.");
        return false;
    }
    if (target->getType() == Entity::E_MOB)
        ((Mob *) target)->confuse();
    return true;
case SK_RESCUE:
    if (!target)
        return false;
    if (skillRate(user, target, sk) < percent_dist(generator()))
        Server::sendMessage(user->getSession(), "Failed");
    else if (target->getType() == Entity::E_MOB)
        ((Mob *) target)->taunt(user);
    target->playEffect(sk->getBase()->effect, 100);
    return true;
default:
    return false;
    }
}

/**
 * \brief Returns the percentage rate of this skill working
 *
 * Returns the percentage rate of this skill working.
 * \param a The entity using the skill
 * \param d The entity which the skill is used on
 * \param sk The skill to check
 * \return The rate of success for the skill, in percents.
 */
unsigned Combat::skillRate(Entity *a, Entity *d, Skill *sk)
{
    int rate;
    int lev = sk->getLevel();
    if (a->getType() == Entity::E_MOB)
        lev = 100;

    const Stats *astat, *dstat;

    switch (sk->getId()) {
    case SK_MOB_ASSAIL:
    case SK_ASSAIL:
    case SK_ASSAULT:
    case SK_CLOBBER:
    case SK_WALLOP:
    case SK_DOUBLE_PUNCH:
    case SK_LONG_STRIKE:
    case SK_THROW_SURIGAM:
    case SK_THRASH:
    case SK_TRIPLE_KICK:
    case SK_MIDNIGHT_SLASH:
    case SK_ASSASSINS_STRIKE:
        //Guessed formula -
        //Observations: l1 mob hits l1 player 67%
        //l1 player hits l1 mob 50%
        //l99 with l1 assail hits about 50% on mobs at level, 100% on low level
        //l99 with 229 dex 23 hit crits ~68%
        //ard cradh increases hit rate
        //hit rate = (ac+100)/4 + skill_level/6 + atk_level/2 + hit/2 - defdex/6
        //crit rate = (dex-lev/3) / 3
        //rate = 67;
        astat = a->getStats();
        dstat = d->getStats();
        if (astat == STATS_INVULNERABLE || dstat == STATS_INVULNERABLE)
            return 0;
        /*rate += (astat->getDex() + astat->getHit() - dstat->getDex()) >> 1;

         rate = rate * (75 + (lev>>2)) / 100;
         rate = rate <= 100 ? rate : 100;*/
        rate = (dstat->getAc() + 100) >> 1;
        rate += (lev - dstat->getDex()) / 3 + a->getLevel() + astat->getHit();
        rate = rate <= 200 ? rate >> 1 : 100;

        rate -= dstat->getDodge();
        return rate > 0 ? rate : 1;
    case SK_WIND_BLADE:
    case SK_KICK:
    case SK_HIGH_KICK:
    case SK_POISON_PUNCH:
    case SK_MANTIS_KICK:
    case SK_EAGLE_STRIKE:
    case SK_WOLF_FANG_FIST:
    case SK_DRACO_TAIL_KICK:
    case SK_KELBEROTH_STRIKE:
    case SK_DARK_SPEAR:
    case SK_STING:
    case SK_RESCUE:
    case SK_THROW:
    case SK_AMBUSH:
    case SK_BEAG_SUAIN:
    case SK_STAB_AND_TWIST:
    case SK_AMNESIA:
    case SK_TRANSFER_BLOOD:
    case SK_CHARGE:
    case SK_BACKSLIDE:
    case SK_TWISTER_STRIKE:
    case SK_CYCLONE_BLADE:
    case SK_SHADOW_FIGURE:
    case SK_STAB_TWICE:
        //TODO check formulas
        return 33 + (lev * 67 / 100);
    case SK_CRASHER:
        if (a->getHp() > a->getMaxHp() / 50)
            return 0;
        return 50 + (lev * 35 / 100);
    case SK_MAD_SOUL:
    case SK_WHIRLWIND_ATTACK:
    case SK_ASGALL_FAILEAS:
        return 25 + (lev * 75 / 100);
    case SK_LUCKY_HAND:
    case SK_MEND_WEAPON:
    case SK_TAILOR:
    case SK_MEND_SOORI:
    case SK_MEND_GARMENT:
        return 25 + (lev >> 1);
    case SK_TWISTER:
        return lev ? 100 : 0;
    case SK_AO_BEAG_SUAIN:
        return 30 + (lev * 60 / 100);
    case SK_BEAG_SRAD:
    case SK_BEAG_CREAG:
    case SK_BEAG_ATHAR:
    case SK_BEAG_SAL:
    case SK_BEAG_SRAD_L:
    case SK_BEAG_CREAG_L:
    case SK_BEAG_ATHAR_L:
    case SK_BEAG_SAL_L:
    case SK_SRAD:
    case SK_CREAG:
    case SK_ATHAR:
    case SK_SAL:
    case SK_SRAD_L:
    case SK_CREAG_L:
    case SK_ATHAR_L:
    case SK_SAL_L:
    case SK_MOR_SRAD:
    case SK_MOR_CREAG:
    case SK_MOR_ATHAR:
    case SK_MOR_SAL:
    case SK_SRAD_MEALL:
    case SK_CREAG_MEALL:
    case SK_ATHAR_MEALL:
    case SK_SAL_MEALL:
    case SK_ARD_SRAD:
    case SK_ARD_CREAG:
    case SK_ARD_ATHAR:
    case SK_ARD_SAL:
    case SK_SRAD_GAR:
    case SK_CREAG_GAR:
    case SK_ATHAR_GAR:
    case SK_SAL_GAR:
    case SK_STRIOCH_BAIS:
    case SK_MOR_STRIOCH_BAIS:
    case SK_MOR_STRIOCH_BAIS_LAMH:
    case SK_MOR_STRIOCH_BAIS_MEALL:
    case SK_TAUNT:
    case SK_BEAG_NOCHD:
    case SK_HOWL:
    case SK_WRAITH_TOUCH:
    case SK_SNORT:
    case SK_SPIT:
    case SK_INSULT:
    case SK_DEO_LAMH:
    case SK_DEO_SAIGHEAD:
    case SK_DEO_SEARG:
    case SK_DEO_SEARG_GAR:
        return 75 + (lev * 25 / 100);
    case SK_SRAD_NADUR:
    case SK_CREAG_NADUR:
    case SK_ATHAR_NADUR:
    case SK_SAL_NADUR:
    case SK_MOR_SRAD_NADUR:
    case SK_MOR_CREAG_NADUR:
    case SK_MOR_ATHAR_NADUR:
    case SK_MOR_SAL_NADUR:
    case SK_ARD_SRAD_NADUR:
    case SK_ARD_CREAG_NADUR:
    case SK_ARD_ATHAR_NADUR:
    case SK_ARD_SAL_NADUR:
        if (a->getLevel() <= d->getLevel())
            return 0;
        /* no break */
    case SK_PIAN_NA_DION:
    case SK_BEAG_CRADH:
    case SK_DACHAIDH:
    case SK_CRADH:
    case SK_MOR_CRADH:
    case SK_ARD_CRADH:
    case SK_CLAW_FIST:
    case SK_KELBEROTH_STANCE:
    case SK_CATS_HEARING:
    case SK_WHITE_BAT_STANCE:
    case SK_TRANCE:
    case SK_ARMACHD:
    case SK_ARMACHD_FEIN:
    case SK_BEAG_PUINNEAG_SPIORAD:
    case SK_PUINNEAG_SPIORAD:
    case SK_MOR_PUINNEAG_SPIORAD:
    case SK_ARD_PUINNEAG_SPIORAD:
    case SK_SPION_BEATHACH:
    case SK_AO_BEAG_CRADH:
    case SK_AO_CRADH:
    case SK_AO_MOR_CRADH:
    case SK_AO_ARD_CRADH:
    case SK_AO_SUAIN:
    case SK_BEANNAICH:
    case SK_MOR_BEANNAICH:
    case SK_FAS_DEIREAS:
    case SK_CREAG_NEART:
    case SK_BEAG_NAOMH_AITE:
    case SK_NAOMH_AITE:
    case SK_MOR_NAOMH_AITE:
    case SK_ARD_NAOMH_AITE:
    case SK_BEAG_BREISLEICH:
    case SK_BREISLEICH:
    case SK_MOR_BREISLEICH:
    case SK_BEAG_SEUN:
    case SK_SEUN:
    case SK_MOR_SEUN:
    case SK_ARD_SEUN:
    case SK_BEAG_PUINSEIN:
    case SK_PUINSEIN:
    case SK_BEAG_PRAMH:
    case SK_PRAMH:
    case SK_DALL:
    case SK_SUAIN:
    case SK_HIDE:
    case SK_EISD_CREUTAIR:
    case SK_MIST:
    case SK_AO_PUINSEIN:
    case SK_AO_DALL:
    case SK_INNER_FIRE:
    case SK_SCORPION_STANCE:
        return 50 + (lev >> 1);
    case SK_PUINNEAG_BEATHA:
        if (d->getHp() <= 2000)
            return 50 + (lev >> 1);
        return 0;
    case SK_DION:
    case SK_DRACO_STANCE:
    case SK_MOR_DION:
    case SK_WINGS_OF_PROTECTION:
    case SK_IRON_SKIN:
    case SK_FAS_SPIORAD:
        return 33 + (lev * 17 / 100);
    case SK_MOR_DION_COMLHA:
        return 35 + (lev * 2 / 5);
    case SK_ROAR:
        return 1 + lev / 11;
    default:
        return 100;
    }

    return 0;
}

/**
 * \brief Returns the damage or heal amount for the skill given
 *
 * Returns the damage or the heal amount for the skill given.
 * \param a The entity using the skill
 * \param d The entity which the skill is being used on
 * \param sk The skill which is being used
 * \return The damage or heal amount for the skill
 */
float Combat::skillDmg(Entity *a, Entity *d, Skill *sk)
{
    float multiplier = 1.0;
    float dmg;
    int atkLev;

    const Stats *atk = a->getStats();
    const Stats *def;
    
    std::uniform_int_distribution<int> weapon_dist_s(atk->getAtkMin(),
						     atk->getAtkMax());
    std::uniform_int_distribution<int> weapon_dist_l(atk->getAtkMinL(),
						     atk->getAtkMaxL());
    std::uniform_int_distribution<int> percent_dist(0, 99);
    std::uniform_real_distribution<double> st_uniform_dist;

    if (d) {
        //Elemental modifier
        int dmod = sk->getElemMod() >= 0 ? sk->getElemMod() : a->getAtkEle();
        multiplier = GET_ELEMENT_MOD(dmod, d->getDefEle(), d->getFasLevel());
        def = d->getStats();
        multiplier *= def == STATS_INVULNERABLE ? 0.0 : def->getAcMod();
        multiplier *= d->modDmg();
    }
    atkLev = a->getLevel();

    //Skill modifier
    switch (sk->getId()) {
    double atkRange;
case SK_ASSAIL:
case SK_MOB_ASSAIL:
case SK_CLOBBER:
case SK_WALLOP:
case SK_ASSAULT:
case SK_DOUBLE_PUNCH:
case SK_LONG_STRIKE:
case SK_THROW_SURIGAM:
case SK_THRASH:
case SK_TRIPLE_KICK:
case SK_MIDNIGHT_SLASH:
    if (d->isSmall())
        dmg = weapon_dist_s(generator()) + atk->getDmg() * 3.0;
    else
        dmg = weapon_dist_l(generator()) + atk->getDmg() * 3.0;
    atkLev = atkLev * 8 / 9;
    dmg += (
        atk->getStr() - atkLev > 0 ?
            (1.5 + .045 * sk->getLevel()) * (atk->getStr() - atkLev) : 0.0);
    if (a->getDir() - d->getDir() == 0) //from behind has a 2x multiplier
        multiplier *= 2.0;
    else if (a->getDir() - d->getDir() != 2 && a->getDir() - d->getDir() != -2) //from the side has a 1.5x multiplier
        multiplier *= 1.5;
    if (a->getType() == Entity::E_CHARACTER) {
        if (((Character *) a)->isUnarmed()
            && ((Character *) a)->getBasePath() == Monk)
            dmg += 16.0 + atk->getStr() + atk->getCon();
        if (((Character *) a)->isUnarmed() && a->hasEffect(StatusEffect::SLAN))
            dmg *= 2.0;
    }
    //Test crit
    if (percent_dist(generator()) < (atk->getDex() - a->getLevel() / 3) / 3)
        dmg *= 2.0;

    break;
case SK_WIND_BLADE:
case SK_EAGLE_STRIKE:
    //TODO check formula for damage
    if (d->isSmall())
        dmg = weapon_dist_s(generator());
    else
        dmg = weapon_dist_l(generator());
    dmg += atk->getStr() * 1.5;
    break;
case SK_CYCLONE_BLADE:
    if (d->isSmall())
        dmg = weapon_dist_s(generator());
    else
        dmg = weapon_dist_l(generator());
    dmg += atk->getStr();
    dmg *= 2.0 + 0.03 * sk->getLevel();
    break;
case SK_CRASHER:
    dmg = a->getMaxHp() * 2.0;
    a->damage(a, a->getHp() - 1);
    break;
case SK_MAD_SOUL:
    dmg = a->getHp() * 0.2;
    a->damage(a, a->getHp() * 19 / 20);
    break;
case SK_WHIRLWIND_ATTACK:
    dmg = a->getMaxHp() * 0.5;
    a->damage(a, a->getHp() * 4 / 5);
    break;
case SK_KICK:
case SK_HIGH_KICK:
    //dmg = atk->getStr() * atk->getDex();
    //dmg *= 0.0592;
    //Think damage is linear on sum of str and con, this formula is a pretty good fit but not perfect
    dmg = (float) (atk->getStr() + atk->getCon());
    dmg *= (sk->getLevel() * 0.05 + 2.49);
    break;
case SK_POISON_PUNCH:
case SK_WOLF_FANG_FIST:
case SK_DARK_SPEAR:
case SK_STING:
    dmg = (float) (atk->getStr() + atk->getCon());
    dmg *= (sk->getLevel() * 0.05 + 1.00);
    break;
case SK_MANTIS_KICK:
case SK_CHARGE_STRIKE:
    dmg = (float) (atk->getStr() * atk->getCon()) * 0.745;
    break;
case SK_DRACO_TAIL_KICK:
    dmg = (float) (atk->getStr() * atk->getCon()) * 0.895; //TODO check
    break;
case SK_KELBEROTH_STRIKE:
    dmg = (a->getHp() / 10) * 3.0;
    a->struck(0, dmg * 2.0);
    return dmg;
case SK_STAB_TWICE:
    multiplier *= 3.0;
    /* no break */
case SK_STAB_AND_TWIST:
    dmg = (float) (atk->getStr() * atk->getDex()) * 0.165; //TODO check
    break;
case SK_TRANSFER_BLOOD:
    dmg = (float) (a->getHp() / 20);
    a->struck(0, dmg);
    break;
case SK_ASSASSINS_STRIKE:
{
    //Assassin's strike should do damage as though for 100 assails with double the modifiers
    //Here, a normal distribution is used instead of rolling 100 assails
    //In particular, the sample mean should be N(average assail damage, uniform dmg deviation / 10)
    if (d->isSmall()) {
        dmg = (atk->getAtkMin() + atk->getAtkMax()) / 2.0 +
	    atk->getDmg() * 3.0;
        atkRange = atk->getAtkRange() / 34.64101615;
    }
    else {
        dmg = (atk->getAtkMinL() + atk->getAtkMaxL()) / 2.0 +
            atk->getDmg() * 3.0;
        atkRange = atk->getAtkRangeL() / 34.64101615;
    }
    
    atkLev = atkLev * 8 / 9;
    dmg += (
        atk->getStr() - atkLev > 0 ?
	(1.5 + .045 * sk->getLevel()) * (atk->getStr() - atkLev) : 0.0);
    //Hit and crit rates
    const float crit_rate = (std::max)
	((atk->getDex() - a->getLevel() * 0.333) * 0.00333, 0.0);
    dmg = dmg * skillRate(a, d, sk)
	* (1.0 + crit_rate);
    atkRange *= skillRate(a, d, sk)
	* (1.0 + crit_rate);

    std::normal_distribution<double> dist(dmg, atkRange);
    dmg = (std::min)(dist(generator()), dmg * 100.0);
    dmg = (std::max)(1.0f, dmg);
    multiplier *= multiplier;
    break;
}
case SK_BULLRUSH_STRIKE:
    dmg = (atk->getStr() * (16.0 + 0.345 * atk->getStr()))
        * (0.5 + 0.005 * sk->getLevel());
    break;
case SK_TWISTER:
    dmg = (atk->getStr() * atk->getCon() * 0.3 * sk->getLevel());
    break;
case SK_NEEDLE_TRAP:
    return 80.0 + 0.2 * sk->getLevel();
case SK_BEAG_SRAD:
case SK_BEAG_CREAG:
case SK_BEAG_ATHAR:
case SK_BEAG_SAL:
case SK_BEAG_SRAD_L:
case SK_BEAG_CREAG_L:
case SK_BEAG_ATHAR_L:
case SK_BEAG_SAL_L:
    //dmg = 15.0 + (3.0 + 0.1 * sk->getLevel()) * sqrt(atk->getInt() > atkLev ? atk->getInt() - atkLev : 0);
    //prev is garbage, but i think is the real implementation
    dmg = 10.0 + (1.5 + 0.025 * sk->getLevel()) * atk->getInt();
    break;
case SK_SRAD:
case SK_CREAG:
case SK_ATHAR:
case SK_SAL:
case SK_SRAD_L:
case SK_CREAG_L:
case SK_ATHAR_L:
case SK_SAL_L:
case SK_DEO_LAMH: //no idea how balanced it is to put this here
case SK_SUP_DUBH:
    //TODO check
    dmg = 40.0 + (8.0 + 0.04 * sk->getLevel()) * atk->getInt();
    break;
case SK_MOR_SRAD:
case SK_MOR_CREAG:
case SK_MOR_ATHAR:
case SK_MOR_SAL:
case SK_SRAD_MEALL:
case SK_CREAG_MEALL:
case SK_ATHAR_MEALL:
case SK_SAL_MEALL:
    multiplier *= 3.0;
    /* no break */
case SK_DEO_SAIGHEAD: //same as deo lamh, dont know effect of putting this here
    //TODO check
    dmg = atk->getInt() * (12.0 + 0.08 * sk->getLevel());
    break;
case SK_DEO_SEARG:
    multiplier *= 1.5;
    /* no break */
case SK_ARD_SRAD:
case SK_ARD_CREAG:
case SK_ARD_ATHAR:
case SK_ARD_SAL:
case SK_SRAD_GAR:
case SK_CREAG_GAR:
case SK_ATHAR_GAR:
case SK_SAL_GAR:
case SK_DEO_SEARG_GAR:
case SK_SRAD_SEID:
case SK_SAL_BREIS:
    //TODO check (im sure there is no linear term but it may be better to include)
    dmg = atk->getInt()
        * (20.0 + atk->getInt() * (0.6 + 0.0025 * sk->getLevel()));
    break;
case SK_STRIOCH_BAIS:
    dmg = a->getMp() * 0.145; //TODO check
    //a->decMp(a->getMp());
    break;
case SK_MOR_STRIOCH_BAIS:
case SK_MOR_STRIOCH_BAIS_LAMH:
case SK_MOR_STRIOCH_BAIS_MEALL:
    dmg = a->getMp() * 0.295; //TODO check
    //a->decMp(a->getMp());
    break;
case SK_MOR_STRIOCH_PIAN_GAR:
    //This formula mostly agrees, but its not perfect
    dmg = a->getMp() * (1.155 + (0.05 * a->getMp()) / atk->getMp());
    multiplier *= d->getFasLevel() == 0 ? 1.027 : 1.0;
    break;
case SK_BEAG_PUINNEAG_SPIORAD:
    if (a->getType() == Entity::E_CHARACTER
        && d->getType() == Entity::E_CHARACTER) {
        a->addMp(d->getMp() * 0.25);
        d->addMp(-d->getMp());
    }
    return 0.0;
case SK_PUINNEAG_SPIORAD:
    if (a->getType() == Entity::E_CHARACTER
        && d->getType() == Entity::E_CHARACTER) {
        a->addMp(d->getMp() * 0.4);
        d->addMp(-d->getMp());
    }
    return 0.0;
case SK_MOR_PUINNEAG_SPIORAD:
    if (a->getType() == Entity::E_CHARACTER
        && d->getType() == Entity::E_CHARACTER) {
        a->addMp(d->getMp() * 0.7);
        d->addMp(-d->getMp());
    }
    return 0.0;
case SK_ARD_PUINNEAG_SPIORAD:
    if (a->getType() == Entity::E_CHARACTER
        && d->getType() == Entity::E_CHARACTER) {
        a->addMp(d->getMp());
        d->addMp(-d->getMp());
    }
    return 0.0;
case SK_PIAN_NA_DION:
    return 12500.0 + 5000.0 * st_uniform_dist(generator());
case SK_DACHAIDH:
    if (a->getType() == Entity::E_CHARACTER)
        if (!DataService::getService()->tryChangeMap((Character *) a, START_MAP,
            START_X, START_Y, 3))
            DataService::getService()->tryChangeMap((Character *) a, START_MAP,
                START_X, START_Y, 0);
    return 0.0;
case SK_BEAG_IOC:
case SK_BEAG_IOC_COMLHA:
case SK_BEAG_IOC_FEIN:
    return 30.0 + 0.7 * sk->getLevel();
case SK_IOC:
case SK_IOC_COMLHA:
    //healed 5848 at 215 wis and level 80 - 27.2*wis? possibly from 0.265 * lev + 6
    return atk->getWis() * (6.0 + 0.265 * sk->getLevel());
case SK_MOR_IOC:
case SK_MOR_IOC_COMLHA:
    return atk->getWis() * (40.0 + sk->getLevel());
case SK_ARD_IOC:
    return atk->getWis() * (60.0 + 1.5 * sk->getLevel());
case SK_NUADHAICH:
    return atk->getWis() * atk->getWis() * 4.0;

case SK_STILETTO_TRAP:
    return 500.0 + sk->getLevel();
case SK_COILED_BOLT_TRAP:
case SK_BOLT_TRAP:
    return 1800.0 + (sk->getLevel() << 1);
case SK_SPRING_TRAP:
    return 5000.0 + 10.0 * sk->getLevel();
case SK_MAIDEN_TRAP:
    return 13000.0 + 20.0 * sk->getLevel();

case SK_TAUNT:
    dmg = 1.0;
    break;
case SK_HOWL:
case SK_SNORT:
case SK_SPIT:
case SK_INSULT:
    dmg = 10.0;
    break;
case SK_PUINNEAG_BEATHA:
    return 2000.0;
case SK_GENTLE_TOUCH:
    return atk->getWis() * (6.0 + 0.315 * sk->getLevel());
case SK_BEAG_NOCHD:
    //TODO check
    dmg = 400.0 + (80.0 + 0.7 * sk->getLevel()) * atk->getInt();
    break;
case SK_WRAITH_TOUCH:
    //TODO check
    dmg = atk->getInt() * atk->getInt() * (0.6 + 0.003 * sk->getLevel());
    break;

case SK_MORNING_STAR:
case SK_EVENING_STAR:
    dmg = 2400000.0;
    break;
case SK_SRAD_TUT:
    dmg = 5.0;
    break;
default:
    //assert(not "Reachable");
    dmg = 0.0;
    break;
    }

    return multiplier * dmg;
}

/**
 * \brief a attempts to use sk to dispel one or more buffs from d
 *
 * Generally dispels work on kinds of buffs, but ao cradh works on specific buffs.
 * \param a [in] The entity using the dispel
 * \param d [in] The entity being dispelled
 * \param sk [in] The skill being used to dispel
 * \return True if a buff was dispelled
 */
bool Combat::dispel(Entity *a, Entity *d, Skill *sk)
{

    switch (sk->getId()) {
    case SK_AO_PUINSEIN:
        return d->removeEffect(StatusEffect::POISON);
    case SK_AO_DALL:
        return d->removeEffect(StatusEffect::BLIND);
    case SK_AO_SUAIN:
        return d->removeEffect(StatusEffect::FREEZE);
    case SK_AO_BEAG_CRADH:
        //TODO fail messages
        return d->removeEffectId(SE_BEAG_CRADH);
    case SK_AO_CRADH:
        return d->removeEffectId(SE_CRADH);
    case SK_AO_MOR_CRADH:
        return d->removeEffectId(SE_MOR_CRADH);
    case SK_AO_ARD_CRADH:
        return d->removeEffectId(SE_ARD_CRADH);
    case SK_REVIVE:
        return d->removeEffect(StatusEffect::DOOM);
    }

    return false;
}
