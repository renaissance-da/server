/*
 * Combat.h
 *
 *  Created on: 2013-02-23
 *      Author: per
 */

#ifndef COMBAT_H_
#define COMBAT_H_

#include "skills.h"
#include "Skill.h"
#include "Stats.h"
#include "Character.h"
#include "srv_proto.h"
#include "DataService.h"
#include "npc_bindings.h"
#include "core.h"
#include "log4cplus/loggingmacros.h"
#include "random.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace Combat
{
void getTargets(Entity *a, Skill *sk, Entity *tgt,
    std::vector<Entity *> &targets);
template<typename E>
bool useSkill(E *a, Skill *sk, Entity *tgt = 0);
unsigned skillRate(Entity *a, Entity *d, Skill *sk);
float skillDmg(Entity *a, Entity *d, Skill *sk);
bool dispel(Entity *a, Entity *d, Skill *sk);
bool specialEffect(Character *user, Skill *sk, std::vector<Entity *> &targets);

template<typename E>
void sendMessage(E *r, const char *msg, char channel = 3);

template<>
void sendMessage(Character *r, const char *msg, char channel);

}

template<> inline
void Combat::sendMessage(Character *r, const char *msg, char channel)
{
    Server::sendMessage(r->getSession(), msg, channel);
}

template<typename E> inline
void Combat::sendMessage(E *r, const char *msg, char channel)
{
    //do nothing
}

/**
 * \brief Causes the entity a to use the skill sk
 *
 * Causes the entity a to use the skill sk. If tgt is given, then
 * sk is used on tgt, provided that a is allowed to do so.
 * \param a The entity who will use the skill
 * \param sk The skill to be used
 * \tgt The target of the skill. This should only be used for targeted
 * 		spells
 * \return True if the skill was improved, false otherwise
 */
template<typename E>
bool Combat::useSkill(E *a, Skill *sk, Entity *tgt)
{
    //Main worker for skill/secret use
    //Skills and secrets generally effect hp/mp of the user and targets
    //They may or may not cause the hp bar to show. They may or may not cause
    //status effects. They may or may not use the target parameter

    //Check if map bars skills
    Map *m = a->getMap();
    if (m->noSkills()) {
        sendMessage(a, "You can't use skills here.");
        return false;
    }

    //Check for statuses preventing action
    if (!a->canAct() && sk->getId() != SK_AO_SUAIN) {
        sendMessage(a, "You can't move.");
        return false;
    }

    //Animate the skill?
    bool no_fail = sk->getFlags() & ALWAYS_SHOW;
    bool animate = no_fail;

    //Check mp requirement
    if (a->getMp() < sk->getBase()->mp) {
        sendMessage(a, "Your will is too weak.");
        return false;
    }

    //Find targets, get role etc
    std::vector<Entity *> targets;
    getTargets(a, sk, tgt, targets);

    if ((sk->getFlags() & SPECIAL_EFFECT)
        && a->getType() == Entity::E_CHARACTER) {
        if (specialEffect((Character *) a, sk, targets)) {
            if (sk->isSecret()) {
                char buffer[100];
                snprintf(buffer, 100, "You cast %s.", sk->getName().c_str());
                sendMessage(a, buffer);
            }
            a->addMp(-sk->getBase()->mp);
            return true;
        }
        return false;
    }
    else if (sk->getFlags() & SCRIPTED) {
        if (a->getType() == Entity::E_CHARACTER) {
            //TODO this shouldn't require being a character
            bool ret = doSpell(((Character *) a), sk->getId());
            char buffer[100];
            snprintf(buffer, 100, "You cast %s", sk->getName().c_str());
            sendMessage(a, buffer);
            return ret;
        }
        else {
            LOG4CPLUS_WARN(core::log(),
                "Non-player used scripted skill " << sk->getId() << ", which is not supported yet.");
            return false;
        }
    }

    //No special effect
    bool chargeMp = false;
    Entity::EntityType tatk = a->getType();
    for (auto it = targets.begin(); it != targets.end(); it++) {
        //Use sk on it
        //Is the target valid?
        Entity::EntityType tdef = (*it)->getType();

        if (sk->getFlags() & OFFENSIVE) {
            if (tatk == Entity::E_MOB && tdef == Entity::E_MOB)
                continue;
            else if (tatk == Entity::E_CHARACTER && !m->pvpOn()
                && tdef == Entity::E_CHARACTER)
                continue;
            else if (a == (*it))
                continue;
        }

        Entity *rec = (*it);

        //Does the spell get deflected?
        if (sk->getFlags() & MISS) {
            if (rec->getStats() == STATS_INVULNERABLE
                || rec->getStats()->getMr() > (random() % 10)) {
                rec->playEffect(0x21, 100);
                sendMessage(a, "The spell has been deflected.");
                continue;
            }
        }
        chargeMp = true;

        //Does the skill work?
        if (skillRate(a, (*it), sk) <= random() % 100) {
            if (!no_fail)
                sendMessage(a, "Failed.");
            if (sk->getFlags() & MISS)
                (*it)->playEffect(0x21, 100);
            continue;
        }

        //Reflect?
        if ((sk->getFlags() & DEFLECT)
            && rec->hasEffect(StatusEffect::ATTACK_REFLECT))
            if ((random() % 100) < 70)
                rec = a;

        //Magic reflect
        if (sk->getTargetType() == TARGET_TARGET
            && rec->hasEffect(StatusEffect::MAGIC_REFLECT))
            if ((random() % 100) < 30)
                rec = a;

        tdef = rec->getType();

        //It worked
        if (sk->getFlags() & DISPEL) {
            dispel(a, rec, sk);
        }

        //TODO this section still needs to be cleaned up
        if (sk->getFlags() & OFFENSIVE) {
            float dmg = skillDmg(a, rec, sk);
            //TODO maybe drain attacks should be handled elsewhere
            if (dmg != 0.0) {
                if (sk->getFlags() & SHOWHP)
                    rec->struck(a, dmg);
                else
                    rec->damage(a, dmg);
            }
        }
        else if (sk->getFlags() & SHOWHP) //TODO add heal flag
            rec->heal(skillDmg(a, rec, sk));

        if (sk->isAssail()) {
            //Assails lower durability
            rec->gearStruck();
            //As well as the attacker's weapon durability
            a->weaponStruck();
        }

        //If the spell is a buff, only animate if the buff is applied successfully
        if (sk->getBase()->buff) {
            if (rec->addStatusEffect(sk->getBase()->buff->id)) {
                animate = true;
            }
            else {
                sendMessage(a, "You already cast that spell.");
                continue;
            }
        }
        //otherwise, always animate
        else
            animate = true;

        if (tdef == Entity::E_CHARACTER && a != rec) {
            if (sk->getFlags() & ATTACK) {
                char buffer[100];
                snprintf(buffer, 100, "%s attacks you with %s.",
                    a->getName().c_str(), sk->getName().c_str());
                Server::sendMessage(((Character *) rec)->getSession(), buffer);
            }
            else if (sk->isSecret()) {
                char buffer[100];
                snprintf(buffer, 100, "%s casts %s on you.",
                    a->getName().c_str(), sk->getName().c_str());
                Server::sendMessage(((Character *) rec)->getSession(), buffer);
            }
        }

        if (sk->getBase()->effect)
            rec->playEffect(sk->getBase()->effect, 0x64);

    }

    if (animate) {
        //Unhidden possibly
        if (!sk->getBase()->buff
            || sk->getBase()->buff->kind != StatusEffect::CONCEAL)
            a->removeEffect(StatusEffect::CONCEAL);

        char anim = sk->getAnim();
        if (!anim)
            anim = a->getAtkAnim();
        a->doAction(anim, sk->getBase()->animLen);
        if (sk->getBase()->sound)
            a->makeSound(sk->getBase()->sound);
        if (sk->getBase()->effectSelf)
            a->playEffect(sk->getBase()->effectSelf, 0x64);

        if (sk->isSecret()) {
            char buffer[100];
            snprintf(buffer, 100, "You cast %s.", sk->getName().c_str());
            sendMessage(a, buffer);
        }
    }

    //TODO nicer aftereffects section
    if (sk->getId() == SK_MOR_STRIOCH_PIAN_GAR
        || sk->getId() == SK_MOR_STRIOCH_BAIS_MEALL
        || sk->getId() == SK_MOR_STRIOCH_BAIS_LAMH
        || sk->getId() == SK_MOR_STRIOCH_BAIS
        || sk->getId() == SK_STRIOCH_BAIS) {
        a->addMp(-a->getMp());
    }
    else if (chargeMp && sk->getBase()->mp)
        a->addMp(-sk->getBase()->mp);

    return targets.size();

}

#endif /* COMBAT_H_ */
