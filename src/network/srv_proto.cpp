/*
 * srv_proto.cpp
 *
 *  Created on: 2012-12-17
 *      Author: per
 */

#include "srv_proto.h"
#include "CharacterSession.h"
#include "DAPacket.h"
#include "NPC.h"
#include <algorithm>

#ifdef WIN32
#define snprintf _snprintf
#endif

/**
 * \brief Send a packet for standard (short) npc dialog, optionally with an option list.
 *
 * npcDlg Sends the client a request to display a short dialog with a list of options.
 * The meaning of short here is that the client should not display back and forward buttons.
 * The option list will be displayed as a list of strings to the client. If the client
 * chooses an option, the client's response will be presented as an offset into the options
 * \param s [in] The CharacterSession associated with the client who will receive the data
 * \param e [in] The entity sending this dialog
 * \param msg [in] The message the NPC is 'sending' to the player.
 * \param msgLen [in] The length of the argument "msg".
 * \param opts [in] The number of options the NPC is presenting to the player. Use 0 for none.
 * \param optList [in] The options to be presented to the player, as null-terminated strings.
 */
void Server::npcDlg(CharacterSession *s, Entity *e, const char *msg,
    unsigned short msgLen, char opts, const char **optList)
{
    char data[3] = { 0, 0, 1 };

    DAPacket dlg(NPC_SERVICE, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);

        dlg.appendByte(0);

        dlg.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendShort(msgLen);
    dlg.appendBytes(msgLen, msg);
    dlg.appendByte(opts);
    int nOpts = (int) opts;
    for (int i = 0; i < nOpts; i++) {
        dlg.appendString(strlen(optList[i]), optList[i]);
        dlg.appendShort(i + 1);
    }

    s->sendPacket(&dlg);
}

/**
 * \brief Tells the client that they have left a map
 *
 * Tells the client they have left a map. The client probably ignores this packet,
 * but it is included for compatibility.
 *
 * \param s [in] The CharacterSession associated with the client who will receive the message
 */
void Server::leaveMap(CharacterSession *s)
{
    char data[1] = { 0 };
    DAPacket rep(LEFT_MAP, data, 1);
    rep.appendByte(3);
    rep.appendInt(0);
    s->sendPacket(&rep);
}

/**
 * \brief Tells the client they have entered a map.
 *
 * Tells the client they have entered a map. This message is also used to refresh the
 * player's position.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param mapId [in] The unique ID of the map to be entered. Corresponds to the map file the client will load.
 * \param width [in] The width of the map. Currently limited to 255
 * \param height [in] The height of the map. Currently limited to 255
 * \param checksum [in] The (bugged) crc checksum of the map's data file. The client will request the map if
 * 						it's map file is not present or the checksum is different.
 * \param name [in] The name of the map which will be displayed to the client.
 * \param nameLen [in] The length of the map's name. Should be limited to 25.
 */
void Server::enterMap(CharacterSession *s, unsigned short mapId, char width,
    char height, unsigned short checksum, const char *name, char nameLen)
{
    char data[1];
    DAPacket enterMap(AREA_DATA, data, 1);
    enterMap.appendShort(mapId);
    enterMap.appendByte(width);
    enterMap.appendByte(height);
    enterMap.appendByte(0);
    enterMap.appendShort(0);
    enterMap.appendShort(checksum);
    enterMap.appendString(nameLen, name);
    enterMap.appendByte(0);

    s->sendPacket(&enterMap);
}

/**
 * \brief Tells the client that an entity is no longer around
 *
 * Tells the client an entity is no longer around. The client typically will stop drawing the
 * entity as a result.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param oid [in] The online ID of the entity which has disappeared
 */
void Server::entGone(CharacterSession *s, unsigned int oid)
{
    char data[1];

    DAPacket gone(Server::ENT_GONE, data, 1);
    gone.appendInt(oid);

    s->sendPacket(&gone);
}

/**
 * \brief Tells the client that an viewable is nearby
 *
 * Tells the client that a new viewable is nearby.
 *
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param e [in] The viewable to show
 */
void Server::showViewable(CharacterSession *s, Viewable *v)
{
    if (v->extendedViewType())
        Server::showExtended(s, v);
    else {
        char data[3] = { 0, 0, 1 };
        DAPacket show(Server::SHOW_MOBS, data, 3);
        v->getViewedBlock(&show);

        s->sendPacket(&show);
    }
}

/**
 * \brief Asks the client to specify a quantity.
 *
 * Asks the client to specify a quantity. For example, when selling from a stack of
 * items, the client is usually asked to specify how many of the item will be sold.
 * This function provides the client with such a dialog.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param e [in] The entity asking for a quantity to be specified
 * \param textLen [in] The length of the string text
 * \param text [in] The string to be sent which asks the client to enter a number
 */
void Server::queryAmount(CharacterSession *s, Entity *e, short int textLen,
    const char *text)
{
    char data[3] = { 0, 3, 1 };

    DAPacket dlg(NPC_SERVICE, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);

        dlg.appendByte(0);

        dlg.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendShort(textLen);
    dlg.appendBytes(textLen, text);

    dlg.appendInt(0x010E004F); //0E indicates that a quantity will be given

    s->sendPacket(&dlg);
}

/**
 * \brief Make a custom text query to a client
 *
 * Similar to queryAmount, but uses the long form dialog window.
 * A string is required, giving the formatted message with the quantity
 * entry field (3) included.
 */
void Server::queryCustom(CharacterSession *s, Entity *e, short int textLen,
    const char *text, short int queryLen, const char *queryBox)
{
    char data[3] = { 0, 4, 1 };

    DAPacket dlg(NPC_DIALOG, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendByte(0);
    dlg.appendByte(0); //global chat id goes here
    dlg.appendByte(0);
    dlg.appendByte(0); //no position
    dlg.appendByte(0); //no back
    dlg.appendByte(1); //allow forward
    dlg.appendByte(0); //unknown

    if (e && e->getType() == Entity::E_NPC)
        dlg.appendString(e->getName().length(), e->getName().c_str());
    else
        dlg.appendString(0, "");

    dlg.appendShort(textLen);
    dlg.appendBytes(textLen, text);

    dlg.appendBytes(queryLen, queryBox);

    s->sendPacket(&dlg);
}

/**
 * \brief Tells the client that an entity has acted
 *
 * Tells the client that an entity has acted, by specifying the animation the entity should
 * perform.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param oid [in] The online ID of the entity who is acting
 * \param anim [in] The animation code describing the action to perform
 * \param animLen [in] The amount of time the animation should be performed over (in 10's of milliseconds)
 */
void Server::entActed(CharacterSession *s, unsigned int oid, char anim,
    short animLen)
{
    char data[1];
    DAPacket animate(Server::ANIMATE, data, 1);
    animate.appendInt(oid);
    animate.appendByte(anim);
    animate.appendShort(animLen);

    s->sendPacket(&animate);
}

/**
 * \brief Sends the client info to display another player's profile
 *
 * Sends the client info to display another player's profile. On receiving this
 * the client should show the profile for the other player, so it should be
 * sent in response to the client clicking on that other player.
 * \param s [in] The session associated with the client who will receive the message
 * \param c [in] The character whos profile info will be sent
 */
void Server::profileInfo(CharacterSession *s, Character *c)
{
    char data[1];
    DAPacket profile(Server::PROFILE_INFO, data, 1);
    c->getProfileInfo(&profile);
    s->sendPacket(&profile);
}

/**
 * \brief Tells the client to play a sound
 *
 * Tells the client to play a specific sound.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param sound [in] The sound to play
 */
void Server::playSound(CharacterSession *s, char sound)
{
    char data[2] = { 0, sound };
    DAPacket playSound(Server::PLAY_SOUND, data, 2);

    s->sendPacket(&playSound);
}

/**
 * \brief Tells the client they have been invited to a group.
 *
 * Tells the client they have been invited to a group by the individual with the given name.
 * \param s [in] The session associated with the client who sill receive the message
 * \param nameLen [in] The length of the string name
 * \param name [in] The name of the individual inviting the client to a group.
 */
void Server::groupInvite(CharacterSession *s, unsigned char nameLen,
    const char *name)
{
    char data[2] = { 0, 1 };
    DAPacket invite(Server::GROUP_INVITE, data, 2);
    invite.appendString(nameLen, name);

    s->sendPacket(&invite);
}

/**
 * \brief Tells the client to start playing one of the BGMs
 *
 * Tells the client to start (or restart) the BGM specified. Playing the same bgm as the current
 * one will cause the track to start over, so this method avoids it by checking the session's current
 * bgm state.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param bgm [in] The track to be played
 */
void Server::setBgm(CharacterSession *s, char bgm)
{
    if (bgm == s->bgm() || s->isBgmSet())
        return;

    s->bgm(bgm);

    unsigned char data[3] = { 0, 0xFF, (unsigned char) bgm };
    DAPacket playBgm(Server::PLAY_SOUND, (const char *) data, 3);

    s->sendPacket(&playBgm);
}

/**
 * \brief Tells the client their coordinates have changed.
 *
 * Tells the client their coordinates have changed. The client's POV will change to the given coordinates,
 * but this is generally overwritten when the server sends the client the attached character's position.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param x [in] The X-coordinate to be set
 * \param y [in] The Y-coordinate to be set
 */
void Server::setCoords(CharacterSession *s, unsigned short x, unsigned short y)
{
    char data[1];
    DAPacket setXY(Server::SET_COORD_INFO, data, 1);
    setXY.appendShort(x);
    setXY.appendShort(y);
    setXY.appendInt(0x000B000B); //Not sure what this part does, probably nothing

    s->sendPacket(&setXY);
}

/**
 * \brief Shows the client a character
 *
 * Shows the client a character they couldn't see before. Will try to show any viewable as a character,
 * but is only sure to work if e is actually a character.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param v [in] The viewable to be shown
 */
void Server::showExtended(CharacterSession *s, Viewable *v)
{
    char data[1];
    DAPacket show(Server::SHOW_CHAR, data, 1);
    v->getViewedBlock(&show);

    s->sendPacket(&show);
}

/**
 * \brief Send a list of door statuses to a player
 *
 * Send a list of door statuses to a player. The entity list given will be sent using each entity's
 * viewed function.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param doors [in] The list of doors to send to the session
 */
void Server::sendDoors(CharacterSession *s, const std::vector<Door *> &doors)
{
    char data[2] = { 0, (char) doors.size() };
    DAPacket show(Server::DOOR_STATUS, data, 2);
    for (unsigned int i = 0; i < doors.size(); i++) {
        doors[i]->getViewedBlock(&show);
    }

    s->sendPacket(&show);
}

/**
 * \brief Send a door status to a player
 *
 * Send a door status to a player.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param d [in] The door which has changed state
 */
void Server::showDoor(CharacterSession *s, Door *d)
{
    char data[2] = { 0, 1 };
    DAPacket show(Server::DOOR_STATUS, data, 2);
    d->getViewedBlock(&show);

    s->sendPacket(&show);
}

void Server::sendFile(CharacterSession *s, const unsigned char *file,
    unsigned char w, unsigned char h)
{
    char data[1] = { 0 };
    for (unsigned short y = 0; y < h; y++) {
        DAPacket *next = new DAPacket(Server::SEND_FILE, data, 1);
        next->appendShort(y);
        for (unsigned short x = 0; x < w * 3; x++) {
            next->appendByte(file[y * w * 6 + x * 2 + 1]);
            next->appendByte(file[y * w * 6 + x * 2]);
        }
        s->sendPacket(next);
        delete next;
    }
}

/**
 * \brief Sends code 0x58
 *
 * Sends code 0x58. Function currently unknown
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 */
void Server::send0x58(CharacterSession *s)
{
    char data[2] = { 0, 0 };
    DAPacket rep(Server::UNK_7, data, 2);

    s->sendPacket(&rep);
}

/**
 * \brief Tells the client that an entity has turned
 *
 * Tells the client that an entity has turned
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param e [in] The entity which has turned
 */
void Server::entTurned(CharacterSession *s, Entity *e)
{
    char data[1];
    DAPacket rep(Server::TURNED, data, 1);
    rep.appendInt(e->getOid());
    rep.appendByte(e->getDir());

    s->sendPacket(&rep);
}

/**
 * \brief Shows the client a collection of viewables
 *
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param ents [in] The collection of viewables which will be shown to the client
 */
void Server::showViewables(CharacterSession *s,
    const std::vector<Viewable *> &views)
{
    //Partition the viewables by type
    std::vector<Viewable *> basic, extended;
    for (auto v : views) {
        if (v->extendedViewType())
            extended.push_back(v);
        else
            basic.push_back(v);
    }

    if (basic.size()) {
        char data[3] = { 0, 0, (char) basic.size() };
        DAPacket showViews(Server::SHOW_MOBS, data, 3);
        for (auto v : basic) {
            v->getViewedBlock(&showViews);
        }

        s->sendPacket(&showViews);
    }

    std::for_each(extended.begin(), extended.end(), [&](Viewable *v) {
        showExtended(s, v);
    });
}

/**
 * \brief Tells the client that an entity moved
 *
 * Tells the client that an entity moved. The entity should be located at its origin, not at the
 * result of its move, when this is called.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param e [in] The entity which is moving
 * \param dir [in] The direction in which the entity is moving
 */
void Server::entMoved(CharacterSession *s, Entity *e, char dir)
{
    char data[2] = { 0, 0 };
    DAPacket movPacket(Server::ENT_MOVE, data, 1);
    movPacket.appendInt(e->getOid());
    movPacket.appendShort(e->getX());
    movPacket.appendShort(e->getY());
    movPacket.appendByte(dir);

    s->sendPacket(&movPacket);
}

/**
 * \brief Tells the client to start a cool-down for one of their skills
 *
 * Tells the client to start a cool-down for one of their skills. The client prevents the player from
 * using a skill during the cool-down period given, but this may differ from the server-side cooldown
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param pane [in] The pane which the skill is found in. Pane 1 is common skills, 0 is secrets
 * \param slot [in] The slot of the pane which the skill is found in, in the range 1..35
 * \param cd [in] The cooldown of the skill, in seconds
 */
void Server::startCd(CharacterSession *s, char pane, char slot, unsigned int cd)
{
    char data[3] = { 0, pane, slot };
    DAPacket setCd(Server::SET_CD, data, 3);
    setCd.appendInt(cd);

    s->sendPacket(&setCd);
}

/**
 * \brief Tells the client to erase a skill
 *
 * Tells the client to erase a skill. Used when a skill is forgotten or moved.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param slot [in] The slot where the skill was found, in the range 1..35
 */
void Server::eraseSkill(CharacterSession *s, char slot)
{
    char data[3] = { 0, slot, 0 };
    DAPacket rep(Server::ERASE_SKILL, data, 3);

    s->sendPacket(&rep);
}

/**
 * \brief Tells the client to erase a secret
 *
 * Tells the client to erase a secret. Used when a secret is forgotten or moved.
 * \param s [in] The CharacterSession associated with the Client who will receive the message
 * \param slot [in] The slot where the secret was found, in the range 1..35
 */
void Server::eraseSecret(CharacterSession *s, char slot)
{
    char data[3] = { 0, slot, 0 };
    DAPacket rep(Server::ERASE_SECRET, data, 3);

    s->sendPacket(&rep);
}

/**
 * \brief Gives the client a skill
 *
 * Gives the client the specified skill. Used when learning a skill, logging in, or moving skills
 * around.
 * \param s [in] The CharacterSession associated with the Client who will receive the message.
 * \param sk [in] The skill which the player is receiving
 * \param slot [in] The slot which the skill is assigned to
 */
void Server::getSkill(CharacterSession *s, Skill *sk, char slot)
{
    char data[4] = { 0, slot, 0 };
    DAPacket rep(Server::SKILL, data, 3);
    sk->getSkillInfo(&rep);

    s->sendPacket(&rep);
}

/**
 * \brief Gives the client a secret.
 *
 * Give the client the specified secret.
 * \param s The session associated with the client who will receive the message.
 * \param sc The secret which the player is receiving.
 * \param slot The slot which the secret is assigned to.
 */
void Server::getSecret(CharacterSession *s, Secret *sc, char slot, int weapon)
{
    char data[3] = { 0, slot, 0 };
    DAPacket rep(Server::GET_SECRET, data, 3);
    sc->getSecretInfo(&rep, weapon);

    //Format, byte(icon) byte(maybe target type) string(name) string(file) byte(lines)

    s->sendPacket(&rep);
}

/**
 * \brief Tells the client about a status effect they are receiving
 *
 * Tells the client about a status effect they are receiving.
 * \param s [in] The session associated with the client who will receive the message
 * \param effect [in] The effect icon
 * \param dur [in] The duration in 20 sec increments, between 0 and 6
 */
void Server::sendStatusEffect(CharacterSession *s, unsigned short effect,
    unsigned char dur)
{
    char data[1];
    DAPacket stEffect(Server::STATUS_EFFECT, data, 1);
    stEffect.appendShort(effect);
    stEffect.appendByte(dur);
    stEffect.appendByte(0);

    s->sendPacket(&stEffect);
}

/**
 * \brief Displays text spoken by another player.
 *
 * Displays some text spoken by another player. The means of displaying the text
 * is determined by the channel
 * \param s The session associated with the user who will receive the message
 * \param oid The online ID of the entity which is talking
 * \param channel The channel for the text. 0 = normal, 1 = shouted, 2 = chanted
 * \param text The text to be shown
 * \param textLen The length of text
 */
void Server::talked(CharacterSession *s, int oid, char channel,
    const char *text, int textLen)
{
    char data[2] = { 0, channel };
    DAPacket talk(Server::TALK, data, 2);

    talk.appendInt(oid);
    talk.appendString(textLen, text);

    s->sendPacket(&talk);
}

/**
 * \brief Request a client to send a stay alive packet
 *
 * Request a client to send a stay alive packet. Can be used to test if
 * a connection has been interrupted messily.
 * \param[in] s The session to be notified
 */
void Server::sendStayAlive(CharacterSession *s)
{
    char data[1] = { 0 };
    DAPacket alive(Server::STAY_ALIVE, data, 1);
    alive.appendShort(0x5544);

    s->sendPacket(&alive);
}

/**
 * \brief Tells the client they have moved
 *
 * Tells the client they have moved. Sent after the client requests a move and is successful
 * \param s [in] The CharacterSession associated with the Client who will receive the message.
 * \param x [in] The player's origin
 * \param y [in] The player's origin
 * \param dir [in] The direction the player moved in. A direction of 4 indicates the move failed
 */
void Server::movedSelf(CharacterSession *s, unsigned short x, unsigned short y,
    char dir)
{
    char data[1];
    DAPacket movedSelf(Server::MOVED_SELF, data, 1);
    movedSelf.appendByte(dir);
    movedSelf.appendShort(x);
    movedSelf.appendShort(y);
    movedSelf.appendInt(0x000B000B);

    s->sendPacket(&movedSelf);
}

void Server::entStruck(CharacterSession *s, unsigned int oid, unsigned short hp,
    unsigned char sound)
{
    char data[1];
    DAPacket struck(Server::STRUCK, data, 1);
    struck.appendInt(oid);
    struck.appendShort(hp);
    struck.appendByte(sound);
    struck.appendByte(0);

    s->sendPacket(&struck);
}

void Server::sendMessage(CharacterSession *s, const char *msg,
    unsigned char channel)
{
    s->systemMessage(msg, channel);
}

void Server::confirmExchange(CharacterSession *s, bool local)
{
    char data[3] = { 0, 5, (char) (local ? 0 : 1) };
    DAPacket exc(Server::UPDATE_TRADE, data, 3);
    exc.appendString(0x0e, "You exchanged.");
    s->sendPacket(&exc);
}
void Server::cancelExchange(CharacterSession *s, bool local)
{
    char data[3] = { 0, 4, (char) (local ? 0 : 1) };
    DAPacket cancel(Server::UPDATE_TRADE, data, 3);
    cancel.appendString(0x13, "Exchange cancelled.");
    s->sendPacket(&cancel);
}
void Server::exchangeAddItem(CharacterSession *s, bool local, Item *itm,
    char pos)
{
    char data[3] = { 0, 2, (char) (local ? 0 : 1) };
    DAPacket addItem(Server::UPDATE_TRADE, data, 3);
    itm->getTradeView(&addItem, pos);
    addItem.appendShort(0); //have some buffering...
    s->sendPacket(&addItem);
}
void Server::exchangeAddGold(CharacterSession *s, bool local, int amt)
{
    char data[3] = { 0, 3, (char) (local ? 0 : 1) };
    DAPacket addGold(Server::UPDATE_TRADE, data, 3);
    addGold.appendInt(amt);
    s->sendPacket(&addGold);
}
void Server::exchangeHowMany(CharacterSession *s, char slot)
{
    char data[3] = { 0, 1, slot };
    DAPacket howMany(Server::UPDATE_TRADE, data, 3);
    s->sendPacket(&howMany);
}
void Server::startExchangeWith(CharacterSession *s, int oid, const char *name)
{
    char data[3] = { 0, 0 };
    DAPacket startEx(Server::UPDATE_TRADE, data, 2);
    startEx.appendInt(oid);
    startEx.appendString(strlen(name), name);
    s->sendPacket(&startEx);
}

/**
 * \brief Asks the client to request a trade with another entity
 *
 * Asks the client to request a trade with another entity. When the client
 * drags an item onto an entity which is capable of trading, the server can use
 * this to get the client to initiate an exchange
 * \param[in] s The session associated with the user who should start an exchange
 * \param[in] oid The identifier of the entity which the client should begin an
 * 		exchange with
 * \param[in] slot The slot of the item to be used to start the exchange
 */
void Server::initExchange(CharacterSession *s, int oid, unsigned char slot)
{
    char data1[3] = { 0, 0, 6 };
    char data2[3] = { 0, 0, 7 };

    {
        DAPacket beginEx(Server::INIT_TRADE, data1, 3);
        beginEx.appendShort(0x4a00);
        beginEx.appendInt(oid);
        s->sendPacket(&beginEx);
    }

    {
        DAPacket firstItem(Server::INIT_TRADE, data2, 3);
        firstItem.appendShort(0x4a01);
        firstItem.appendInt(oid);
        firstItem.appendByte(slot);
        s->sendPacket(&firstItem);
    }
}

/**
 * \brief Asks the client to request a trade with another entity
 *
 * Asks the client to request a trade with another entity. When the client
 * drags gold onto an entity which is capable of trading, the server can use
 * this to get the client to initiate an exchange
 * \param[in] s The session associated with the user who should start an exchange
 * \param[in] oid The identifier of the entity which the client should begin an
 * 		exchange with
 * \param[in] gold The amount of gold the character should initially offer in the
 * 		exchange
 */
void Server::initGoldExchange(CharacterSession *s, int oid, unsigned int amt)
{
    char data1[3] = { 0, 0, 6 };
    char data2[3] = { 0, 0, 0xA };

    {
        DAPacket beginEx(Server::INIT_TRADE, data1, 3);
        beginEx.appendShort(0x4a00);
        beginEx.appendInt(oid);
        s->sendPacket(&beginEx);
    }

    {
        DAPacket exchGold(Server::INIT_TRADE, data2, 3);
        exchGold.appendShort(0x4a03);
        exchGold.appendInt(oid);
        exchGold.appendInt(amt);
        s->sendPacket(&exchGold);
    }
}

void Server::updateStatInfo(CharacterSession *s, Character *c,
    unsigned int flags)
{
    char data[1];
    DAPacket info(Server::CHAR_DATA, data, 1);
    c->getStatBlock(&info, flags);

    s->sendPacket(&info);
}

/**
 * \brief Plays an effect over the given entity
 *
 * Plays an effect. The effect is centered on the given entity.
 * \param s [in] The session associated with the client who will receive the message.
 * \param oid [in] The online ID of the entity which is receiving the effect.
 * \param anim [in] The animation to play.
 * \param animLen [in] The duration which the animation should play for, in 10's of milliseconds.
 */
void Server::playEffect(CharacterSession *s, unsigned int oid,
			unsigned short anim, unsigned int animLen)
{
    //TODO move this to character
    if (!(s->getCharacter()->getSettings()
        & Character::Settings::BELIEVE_IN_MAGIC))
        return;

    char data[1];
    DAPacket effect(Server::PLAY_EFFECT, data, 1);
    effect.appendInt(oid);
    effect.appendInt(0); //entity who caused it?
    effect.appendShort(anim);
    effect.appendInt(animLen);

    s->sendPacket(&effect);
}

/**
 * \brief Plays an effect at the given location
 *
 * Plays an effect. The effect can be put at an arbitrary location.
 * \param s [in] The session associated with the client who will receive the
 * message.
 * \param x [in] The x coordinate where the effect will be shown.
 * \param y [in] The y coordinate where the effect will be shown.
 * \param anim [in] The animation to play.
 * \param animLen [in] The duration for which the animation should play, in
 * 10's of milliseconds.
 */
void Server::playEffect(CharacterSession *s, unsigned short x, unsigned short y,
			unsigned short anim, unsigned short animLen)
{
    if (!(s->getCharacter()->getSettings()
	  & Character::Settings::BELIEVE_IN_MAGIC))
	return;

    char data[1];
    DAPacket effect(Server::PLAY_EFFECT, data, 1);
    effect.appendInt(0); // Target oid not given
    effect.appendShort(anim);
    effect.appendShort(animLen);
    effect.appendShort(x);
    effect.appendShort(y);
    //effect.appendShort(0); Also originally included, seems to have no effect

    s->sendPacket(&effect);
}

/**
 * \brief Gives the client an item
 *
 * Gives the client an item. The properties of the item, whether it can be
 * stacked and so on, depend only on the values given as far as the client is
 * concerned.
 * \param s [in] The session associated with the client who will receive the
 * message.
 * \param slot [in] The slot which the item will appear in. Slot should be in
 * 1..35 as it corresponds to the slot seen, not the array index.
 * \param qty [in] The number of items in the stack. 0 specifies a non-stacking
 * item.
 * \param name [in] The name of the item, as will be displayed to the client.
 * \param nameLen [in] The length of name, not counting the null terminator.
 * \param apr [in] The appearance code of the item.
 * \param dur [in] The current durability of the item. An item with no
 * durability should set this and max to 0.
 * \param maxDur [in] The max durability of the item.
 */
void Server::getItem(CharacterSession *s, unsigned char slot, unsigned char qty,
		     const char *name, unsigned short nameLen, unsigned short apr,
		     unsigned int dur, unsigned int maxDur)
{
    char data[1];
    DAPacket item(Server::GET_ITEM, data, 1);
    item.appendByte(slot);
    item.appendShort(apr);
    item.appendShort(nameLen);
    item.appendBytes(nameLen, name);
    item.appendInt(qty > 0 ? qty : 1);
    item.appendByte(qty > 0 ? 1 : 0);
    item.appendInt(maxDur);
    item.appendInt(dur);

    s->sendPacket(&item);
}

void Server::equipItem(CharacterSession *s, unsigned char slot,
    const char *name, unsigned short nameLen, unsigned short apr,
    unsigned int dur, unsigned int maxDur)
{
    char data[1];
    DAPacket eqp(Server::EQUIPPED_ITEM, data, 1);
    eqp.appendByte(slot);
    eqp.appendShort(apr);
    eqp.appendShort(nameLen);
    eqp.appendBytes(nameLen, name);
    eqp.appendByte(0);
    eqp.appendInt(maxDur);
    eqp.appendInt(dur);
    eqp.appendByte(0);

    s->sendPacket(&eqp);
}

/**
 * \brief Tells the client to remove an item from the inventory
 *
 * Tells the client to remove an item from the inventory.
 * \param s [in] The session associated with the user who will receive the message.
 * \param slot [in] The slot of the item which will be removed, as presented to the user (ie. in 1..59)
 */
void Server::removeItem(CharacterSession *s, unsigned char slot)
{
    char data[1];
    DAPacket rem(Server::REMOVE_ITEM, data, 1);
    rem.appendByte(slot);
    rem.appendInt(0x0C001100); //purpose unknown

    s->sendPacket(&rem);
}

/**
 * \brief Tells the client an item they had equipped is gone
 *
 * Tells the client an item they had equipped is gone. The client deletes the item from the
 * character pane on receiving.
 * \param s [in] The session associated with the user who will receive the message.
 * \param slot [in] The equipment slot of the item which will be removed, in 1..18?
 */
void Server::removeEquip(CharacterSession *s, unsigned char slot)
{
    char data[1];
    DAPacket rem(Server::EQUIPMENT_GONE, data, 1);
    rem.appendByte(slot);

    s->sendPacket(&rem);
}

/**
 * \brief Sends the client a buying list
 *
 * Sends the client a buying list. The user can use the list to buy an item.
 * \param s [in] The session associated with the client who will receive the message.
 * \param oid [in] The online ID of the npc who is providing the service.
 * \param apr [in] The appearance code of the npc who is providing the service.
 * \param nameLen [in] The length of the npc's name.
 * \param name [in] The npc's name.
 * \param textLen [in] The length of the message the npc will send along with the list.
 * \param text [in] The message the npc will send along with the list.
 * \param items [in] The list of items to be displayed.
 */
void Server::sendVendList(CharacterSession *s, Entity *e, short textLen,
    const char *text, std::vector<BaseItem *> &items)
{
    /* server reply is 2F 04 01 OID 01 40 1E 00 01 40 1E 00 00 name (short textlen) (talk text)
     * 00 4A 00 03 <- make a list of 3
     * short graphic 00 int_cost str_name str_features
     * 80 56 00 00 00 00 64 05 "Stick"  10 "All, Levxx, Wtxx"
     * 80 5b 00 00 00 01 F4 04 "Dirk"
     * 80 59 00 00 00 05 14 0A "Claidheamh" 10 "All, Levxx, Wtxx"
     * 74 20 38 (garbage probably)*/

    char data[3] = { 0, 4, 1 };
    DAPacket vend(Server::NPC_SERVICE, data, 3);
    if (e)
        vend.appendInt(e->getOid());
    else
        vend.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        vend.appendByte(1);
        vend.appendShort(npc->getApr());
        vend.appendByte(0);
        vend.appendByte(1);
        vend.appendShort(npc->getApr());
        vend.appendByte(0);

        vend.appendByte(0);

        vend.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        vend.appendInt(0);
        vend.appendInt(0);
        vend.appendByte(0);
        vend.appendString(0, "");
    }

    vend.appendShort(textLen);
    vend.appendBytes(textLen, text);
    vend.appendShort(0x4A);
    vend.appendShort(items.size());
    for (auto it = items.begin(); it != items.end(); it++) {
        vend.appendShort((*it)->getApr());
        vend.appendByte(0);
        vend.appendInt((*it)->getVal());
        vend.appendString((*it)->getNameLen(), (*it)->getName());
        if ((*it)->getType() == BaseItem::EQUIP) {
            //TODO print feature list
            vend.appendByte(0);
        }
        else
            vend.appendByte(0);
    }

    s->sendPacket(&vend);
}

/**
 * \brief Sends the client a buying list
 *
 * Sends the client a buying list. The user can use the list to buy an item.
 * \param s [in] The session associated with the client who will receive the message.
 * \param oid [in] The online ID of the npc who is providing the service.
 * \param apr [in] The appearance code of the npc who is providing the service.
 * \param nameLen [in] The length of the npc's name.
 * \param name [in] The npc's name.
 * \param textLen [in] The length of the message the npc will send along with the list.
 * \param text [in] The message the npc will send along with the list.
 * \param items [in] The list of items to be displayed.
 */
void Server::sendVendList(CharacterSession *s, Entity *e, short textLen,
    const char *text, const std::vector<Character::StorageItem> &items)
{
    char data[3] = { 0, 4, 1 };
    DAPacket vend(Server::NPC_SERVICE, data, 3);
    if (e)
        vend.appendInt(e->getOid());
    else
        vend.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        vend.appendByte(1);
        vend.appendShort(npc->getApr());
        vend.appendByte(0);
        vend.appendByte(1);
        vend.appendShort(npc->getApr());
        vend.appendByte(0);

        vend.appendByte(0);

        vend.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        vend.appendInt(0);
        vend.appendInt(0);
        vend.appendByte(0);
        vend.appendString(0, "");
    }

    vend.appendShort(textLen);
    vend.appendBytes(textLen, text);
    vend.appendShort(0x4A);
    vend.appendShort(items.size());
    for (auto it = items.begin(); it != items.end(); it++) {
        BaseItem *bi = BaseItem::getById(it->id);
        if (!bi)
            continue;
        vend.appendShort(bi->getApr());
        vend.appendByte(0);
        vend.appendInt(it->qty);

        if (it->mod) {
            char buffer[100];
            int len = snprintf(buffer, 100, "%s %s",
                Equipment::getModName(((BaseEquipment *) bi)->getSlot(),
                    it->mod), bi->getName());
            vend.appendString(len, buffer);
        }
        else
            vend.appendString(bi->getNameLen(), bi->getName());

        if (bi->getType() == BaseItem::EQUIP) {
            //TODO print feature list
            vend.appendByte(0);
        }
        else
            vend.appendByte(0);
    }

    s->sendPacket(&vend);
}

void Server::sendItemSublist(CharacterSession *s, Entity *e, short textLen,
    const char *text, std::vector<char> &slots)
{
    char data[3] = { 0, 5, 1 };
    DAPacket dlg(Server::NPC_SERVICE, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);

        dlg.appendByte(0);

        dlg.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendShort(textLen);
    dlg.appendBytes(textLen, text);

    dlg.appendShort(0x4D);
    dlg.appendByte(slots.size());
    std::for_each(slots.begin(), slots.end(),
        [&](char slot) {dlg.appendByte(slot);});

    s->sendPacket(&dlg);
}

/**
 * Send the country list to the player.
 * \param s The session associated with the client who will receive the message
 * \param list The list of online players, to be shown in the order given
 * \param total The total number online, which includes players in other 'countries'
 * \param lmin The minimum level at which players will appear as orange.
 * \param lmax The maximum level at which players will appear as orange.
 */
void Server::sendCountryList(CharacterSession *s,
    std::vector<Character *> *list, short total, short lmin, short lmax)
{
    char data;
    DAPacket showList(Server::COUNTRY_LIST, &data, 1);

    showList.appendShort(total);
    showList.appendShort(list->size());
    std::for_each(list->begin(), list->end(), [&](Character *c) {
        showList.appendByte(c->getBasePath() + (c->isMaster() ? 0x80 : 0x10));
        if (c->getLevel() >= lmin && c->getLevel() <= lmax)
        showList.appendByte(0x97);
        else
        showList.appendByte(0xff);
        showList.appendShort(0);
        showList.appendByte(c->isMaster());
        showList.appendString(c->getName().length(), c->getName().c_str());
    });

    s->sendPacket(&showList);
}

void Server::showBoard(CharacterSession *s, std::vector<const char *> *opts)
{
    //TODO implement
    char data;
    DAPacket boards(Server::SHOW_BOARD, &data, 1);
    boards.appendByte(1);
    boards.appendByte(0); //think board number and # items
    boards.appendByte(opts->size());
    for (unsigned short i = 0; i < opts->size(); i++) {
        boards.appendShort(i);
        boards.appendString(strlen((*opts)[i]), (*opts)[i]);
    }

    s->sendPacket(&boards);
}

void Server::endSignal(CharacterSession *s)
{
    char data[4] = { 0, 1, 0, 0 };
    DAPacket end(Server::END_SIGNAL, data, 4);

    s->sendPacket(&end);
}

/**
 * \brief Sent to inform a client of which settings they are using.
 *
 * Informs a client of the server-side settings associated with the character
 * they are using. These settings show up on the client as the options 1-8 in
 * the F4 menu.
 * \param[in] s The session associated with the client to be notified.
 * \param[in] setting The setting of interest. 0 for all of the settings.
 */
void Server::updateSettings(CharacterSession *s, int setting)
{
    char data[2] = { 0, 7 };
    DAPacket dlg(Server::SYSTEM_MESSAGE, data, 2);
    s->getCharacter()->getSettings(&dlg, setting);
    s->sendPacket(&dlg);
}

void Server::sendForgetList(CharacterSession *s, Entity *e, short textLen,
    const char *text, bool secrets)
{
    char data[3] = { 0, (char) (secrets ? 8 : 9), 1 };
    DAPacket dlg(Server::NPC_SERVICE, data, 3);

    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);

        dlg.appendByte(0);

        dlg.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendShort(textLen);
    dlg.appendBytes(textLen, text);
    dlg.appendShort(secrets ? 0x13 : 0x19);

    s->sendPacket(&dlg);
}

void Server::sendSkillList(CharacterSession *s, Entity *e, short textLen,
    const char *text, std::vector<SkillInfo *> &skills,
    bool spellList)
{
    char data[3] = { 0, 6, 1 };
    DAPacket dlg(Server::NPC_SERVICE, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);

        dlg.appendByte(0);

        dlg.appendString(npc->getName().length(), npc->getName().c_str());
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
        dlg.appendByte(0);
        dlg.appendString(0, "");
    }

    dlg.appendShort(textLen);
    dlg.appendBytes(textLen, text);

    dlg.appendShort(spellList ? 0x11 : 0x17);
    dlg.appendShort(skills.size());
    std::for_each(skills.begin(), skills.end(), [&](SkillInfo *sk) {
        dlg.appendByte(spellList ? 2 : 3); //Think 3=skills, 2=spells...
        dlg.appendByte(0);
        dlg.appendByte(sk->icon);
        dlg.appendByte(1);
        dlg.appendString(sk->nameLen, sk->name);
    });

    s->sendPacket(&dlg);
}

/**
 * \brief Sends a list of fields to a client
 *
 * Sends a list of fields to the client. The client will display the fields on the world map
 * \param s The session associated with the client who will receive the message
 * \param name The name of the field, which is currently not visible in the client.
 * \param fieldDests The maps which are accessible from this field
 */
void Server::fieldWarp(CharacterSession *s, const char *name,
    std::vector<Field::FieldDest> &fieldDests)
{
    char data;
    DAPacket field(Server::FIELD_WARP, &data, 1);

    field.appendString(strlen(name), name);
    field.appendByte(fieldDests.size());
    field.appendByte(1); //the map to show?

    for (auto it = fieldDests.begin(); it != fieldDests.end(); it++) {
        field.appendShort(it->x);
        field.appendShort(it->y);
        field.appendString(strlen(it->name), it->name);
        field.appendShort(0);
        field.appendShort(it->mapId);
        field.appendShort(it->xdest);
        field.appendShort(it->ydest);
    }

    s->sendPacket(&field);
}

void Server::sendMetadata(CharacterSession *s, unsigned char metapath)
{
    unsigned char data[] = { 0x00, 0x01, 0x00, 0x17, 0x07, 0x53, 0x43, 0x6C,
        0x61, 0x73, 0x73, 0x33, 0xA7, 0x14, 0x6E, 0xC0, 0x07, 0x53, 0x43, 0x6C,
        0x61, 0x73, 0x73, 0x34, 0xCE, 0x37, 0x38, 0xEA, 0x07, 0x53, 0x43, 0x6C,
        0x61, 0x73, 0x73, 0x35, 0xA8, 0x3B, 0x7D, 0x0E, 0x09, 0x49, 0x74, 0x65,
        0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x30, 0x81, 0x00, 0xD2, 0xA0, 0x09, 0x49,
        0x74, 0x65, 0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x31, 0x4A, 0xB6, 0x17, 0x86,
        0x09, 0x49, 0x74, 0x65, 0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x32, 0x39, 0xB7,
        0x2C, 0x1D, 0x09, 0x49, 0x74, 0x65, 0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x33,
        0x77, 0xC1, 0x20, 0x27, 0x07, 0x53, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x31,
        0x6B, 0xF0, 0xE5, 0x0A, 0x09, 0x49, 0x74, 0x65, 0x6D, 0x49, 0x6E, 0x66,
        0x6F, 0x34, 0x60, 0x20, 0x7A, 0x70, 0x07, 0x53, 0x45, 0x76, 0x65, 0x6E,
        0x74, 0x32, 0xAF, 0x19, 0xCA, 0x7C, 0x09, 0x49, 0x74, 0x65, 0x6D, 0x49,
        0x6E, 0x66, 0x6F, 0x35, 0xF0, 0xBB, 0x4F, 0xAB, 0x07, 0x53, 0x45, 0x76,
        0x65, 0x6E, 0x74, 0x33, 0xFF, 0x1F, 0xC9, 0x95, 0x09, 0x49, 0x74, 0x65,
        0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x36, 0x7F, 0xB5, 0xB9, 0xC9, 0x07, 0x53,
        0x45, 0x76, 0x65, 0x6E, 0x74, 0x34, 0xC1, 0xBC, 0x42, 0xCA, 0x09, 0x49,
        0x74, 0x65, 0x6D, 0x49, 0x6E, 0x66, 0x6F, 0x37, 0xE0, 0x87, 0x38, 0x79,
        0x07, 0x53, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x35, 0x7D, 0x48, 0x86, 0xD2,
        0x07, 0x53, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x36, 0x7E, 0x9E, 0x90, 0x78,
        0x07, 0x53, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x37, 0x7E, 0x9E, 0x90, 0x78,
        0x0A, 0x4E, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x44, 0x65, 0x73, 0x63, 0x10,
        0xCF, 0xF7, 0xF8, 0x05, 0x4C, 0x69, 0x67, 0x68, 0x74, 0x1E, 0x96, 0x1C,
        0x27, 0x09, 0x4E, 0x50, 0x43, 0x49, 0x6C, 0x6C, 0x75, 0x73, 0x74, 0xC2,
        0xF5, 0x03, 0xD9, 0x07, 0x53, 0x43, 0x6C, 0x61, 0x73, 0x73, 0x31, 0xD1,
        0x09, 0xB5, 0x39, 0x07, 0x53, 0x43, 0x6C, 0x61, 0x73, 0x73, 0x32, 0x4B,
        0x41, 0x70, 0x4E, 0x00 };

    DAPacket bla(Server::METADATA, (const char *) data, sizeof(data));

    s->sendPacket(&bla);
}

void Server::sendLongDlg(CharacterSession *s, Entity *e, const char *msg,
    unsigned short msgLen,
    bool next,
    bool prev, char pos, char opts, const char **optList)
{
    char data[3] = { 0, 0, 1 }; // these options can be used to list opts to player

    DAPacket dlg(NPC_DIALOG, data, 3);
    if (e)
        dlg.appendInt(e->getOid());
    else
        dlg.appendInt(0);

    if (e && e->getType() == Entity::E_NPC) {
        NPC *npc = (NPC *) e;
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
        dlg.appendByte(1);
        dlg.appendShort(npc->getApr());
        dlg.appendByte(0);
    }
    else {
        dlg.appendInt(0);
        dlg.appendInt(0);
    }

    dlg.appendByte(0);
    dlg.appendByte(0); //global chat id goes here
    dlg.appendByte(0);
    dlg.appendByte(pos);
    dlg.appendByte(prev ? 1 : 0);
    dlg.appendByte(next ? 1 : 0);
    dlg.appendByte(0);

    if (e && e->getType() == Entity::E_NPC)
        dlg.appendString(e->getName().length(), e->getName().c_str());
    else
        dlg.appendString(0, "");

    dlg.appendShort(msgLen);
    dlg.appendBytes(msgLen, msg);

    if (opts) {
        dlg.appendByte(opts);
        int nOpts = (int) opts;
        for (int i = 0; i < nOpts; i++) {
            dlg.appendString(strlen(optList[i]), optList[i]);
            dlg.appendShort(i + 1);
        }
    }
    s->sendPacket(&dlg);
}

void Server::closeDialog(CharacterSession *s)
{
    char data[3] = { 0, 0xa, 0 };
    DAPacket rep(Server::NPC_DIALOG, data, 2);
    s->sendPacket(&rep);
}
