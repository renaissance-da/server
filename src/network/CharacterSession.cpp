/*
 * CharacterSession.cpp
 *
 *  Created on: 2011-07-22
 *      Author: per
 */

#include "CharacterSession.h"
#include "DAPacket.h"
#include "DataService.h"
#include "defines.h"
#include "Parser.h"
#include <stdio.h>
#include <string.h>
#include "GameTime.h"
#include "srv_proto.h"
#include "dialogHandlers.h"
#include "npc_bindings.h"
#include "config.h"
#include "core.h"
#include "log4cplus/loggingmacros.h"
#include "DataLoaders.h"
#include "MobInfo.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

CharacterSession::CharacterSession(int sockfd, in_addr_t ip, int now):
BasicSession(sockfd, now, ip),
service(0),
character(0),
moveOrdinal(0),
curBgm(1),
bgmPlayerSet(false),
#ifndef NDEBUG
writelock(PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP)
#else
writelock(PTHREAD_MUTEX_INITIALIZER)
#endif
{
	ordinal = 0;
	//Wait for data to arrive from client
}

CharacterSession::~CharacterSession() {
	assert(((character && service) || (!character && !service)));
	if (character) {
		DataService::getService()->freeCharacter(character);
		delete service;
	}
	pthread_mutex_destroy(&writelock);
}

void CharacterSession::sendPacket(DAPacket *p)
{
	lock();
	*(p->getDataPtr()) = ordinal++;
	service->encrypt(p);
	try {
		p->writeData(fd);
	}
	catch(DAPacket::PacketError &pe) {
		if (pe == DAPacket::CONNECTION_CLOSED)
			open = false;
	}
	unlock();
}

void CharacterSession::disconnect()
{
	//Anything else to do here?
	open = false;
}

void CharacterSession::lock()
{
	int r = pthread_mutex_lock(&writelock);
	assert(!r);
}

void CharacterSession::unlock()
{
	int r = pthread_mutex_unlock(&writelock);
	assert(!r);
}

void CharacterSession::aboutToTimeout()
{
    if (service)
	Server::sendStayAlive(this);
}

void CharacterSession::dataReady()
{
	DAPacket *p;
	try {
		p = new DAPacket(fd);
	}
	catch (DAPacket::PacketError &pe) {
		// Here we could ignore bad packets or write to the log, but for now we will just terminate the connection
		if (pe == DAPacket::CONNECTION_CLOSED)
			open = false;
		return;
	}

	if (character == 0 && p->getCode() != Client::ATTACH_CHARACTER) {
		open = false;
		delete p;
		return;
	}

	try {
	switch (p->getCode()) {
	case (Client::ATTACH_CHARACTER):
		attachCharacter(p);
		break;
	case (Client::PING):
		break;
	case (Client::GET_EXTRA_INFO):
		getExtraInfo();
		break;
	case (Client::GET_LEGEND_INFO):
		getLegendData();
		break;
	case (Client::MOVE):
		move(p);
		break;
	case (Client::TALK):
		talk(p);
		break;
	case (Client::LOGOUT):
		logout(p);
		break;
	case (Client::TURN):
		turn(p);
		break;
	case (Client::USE_SKILL):
		useSkill(p);
		break;
	case (Client::ATTACK):
		attack();
		break;
	case (Client::MOVE_PANE):
		movePane(p);
		break;
	case (Client::CLICKED):
		clicked(p);
		break;
	case (Client::REFRESH):
		refresh();
		break;
	case (Client::INC_STAT):
		incStat(p);
		break;
	case (Client::USE_ITEM):
		useItem(p);
		break;
	case (Client::UNEQUIP_SLOT):
		unequipItem(p);
		break;
	case (Client::DROP_ITEM):
		dropItem(p);
		break;
	case (Client::PICKUP):
		pickupItem(p);
		break;
	case (Client::DROP_GOLD):
		dropGold(p);
		break;
	case (Client::GIVE_GOLD):
		giveGold(p);
		break;
	case (Client::WHISPER):
		whisper(p);
		break;
	case (Client::RESUME_CONV):
		resumeConv(p);
		break;
	case (Client::RESUME_CONV_L):
		resumeConvLong(p);
		break;
	case (Client::COUNTRY_LIST):
		countryList();
		break;
	case (Client::READ_BOARD):
		showBoard(p);
		break;
	case (Client::FIELD_WARP):
		fieldWarp(p);
		break;
	case (Client::CHANT):
		chant(p);
		break;
	case (Client::START_CASTING):
		startCasting(p);
		break;
	case (Client::USE_SECRET):
		useSecret(p);
		break;
	case (Client::TOGGLE_GROUP):
		toggleGroup();
		break;
	case (Client::GROUP_INVITE):
		groupInvite(p);
		break;
	case (Client::EMOTE):
		emote(p);
		break;
	case (Client::GIVE_ITEM):
		giveItem(p);
		break;
	case (Client::TRADE):
		trade(p);
		break;
	case (Client::GET_MAP):
		getMap(p);
		break;
	case (Client::SETTINGS):
		settings(p);
		break;
	default:
		//Unrecognised packet code
		//For now, just terminate the connection
		//open = false;
		//TODO uncomment?
		break;
	}}
	catch (DAPacket::PacketError &pe) {
		if (pe == DAPacket::CONNECTION_CLOSED)
			open = false;
	}

	delete p;
}

void CharacterSession::getMap(DAPacket *p)
{
	service->decrypt(p);
	p->skip(4);
	unsigned char width = p->extractByte();
	unsigned char height = p->extractByte();
	Map *m = character->getMap();
	if (width == m->getWidth() && height == m->getHeight())
		Server::sendFile(this, m->getFile(), width, height);
}

void CharacterSession::useSecret(DAPacket *p)
{
	service->decrypt(p);
	unsigned char slot = p->extractByte();
	int oid = 0;
	if (p->getDataLen() >= 5) {
		oid = p->extractInt();
	}
	LockedChar lc = character;
	lc->useSecret(slot, oid);
}

void CharacterSession::giveItem(DAPacket *p)
{
	service->decrypt(p);
	char slot = p->extractByte();
	int oid = p->extractInt();
	if (slot > NUM_ITEMS || slot < 0)
		return;
	LockedChar lc = character;
	lc->giveItem(oid, slot);
}

void CharacterSession::trade(DAPacket *p)
{
	service->decrypt(p);
	char type = p->extractByte();
	int oid = p->extractInt();

	LockedChar lc = character;
	switch (type) {
	case 0:
		//Start trade
		lc->startTrade(oid);
		break;
	case 1:
		//Add item
		lc->tradeItem(oid, p->extractByte());
		break;
	case 2:
		//Add x of an item
		char slot, amt;
		slot = p->extractByte();
		amt = p->extractByte();
		lc->tradeItem(oid, slot, amt);
		break;
	case 3:
		//Add gold
		lc->tradeGold(oid, p->extractInt());
		break;
	case 4:
		//Cancel trade
		lc->cancelTrade();
		break;
	case 5:
		//confirm trade
		lc->confirmTrade();
		break;
	default:
		break;
	}
}

void CharacterSession::startCasting(DAPacket *p)
{
	service->decrypt(p);
	unsigned char time = p->extractByte();

	character->startChant(time);
}

void CharacterSession::settings(DAPacket *p)
{
	service->decrypt(p);
	int op = p->extractByte();

	if (op > 0)
		character->toggleSetting(op);
	Server::updateSettings(this, op);
}

void CharacterSession::fieldWarp(DAPacket *p)
{
	service->decrypt(p);

	p->skip(2);
	short mapId = p->extractShort();
	LockedChar lc = character;
	lc->fieldJump(mapId);
}

void CharacterSession::chant(DAPacket *p)
{
	if (!character->isCasting()) {
		return;
	}
	service->decrypt(p);
	std::string text = p->extractString();
	if (text.length() > 32)
		text = text.substr(0, 32);

	LockedChar lc = character;
	Map *m = lc->getMap();
	m->forEachNearby(lc.get(), [&](Entity *e) {
		e->talked(lc.get(), text, 2);
	});
}

void CharacterSession::toggleGroup()
{
	character->toggleSetting(2);
}

void CharacterSession::groupInvite(DAPacket *p)
{
	service->decrypt(p);
	unsigned char mode = p->extractByte();
	std::string name = p->extractString();

	if (mode == 2)
		DataService::getService()->groupInvite(character, name);
	else if (mode == 3)
		DataService::getService()->groupAccept(character, name);
}

void CharacterSession::emote(DAPacket *p)
{
	service->decrypt(p);

	unsigned char em = p->extractByte();
	if (em > 0x23)
		return;
	em += 9;
	LockedChar lc = character;
	lc->doAction(em, 0x78);
}

void CharacterSession::giveGold(DAPacket *p)
{
	service->decrypt(p);

	unsigned int amt = p->extractInt();
	int oid = p->extractInt();

	LockedChar lc = character;
	lc->giveGold(oid, amt);
}

void CharacterSession::showBoard(DAPacket *p)
{
	//TODO implement
	service->decrypt(p);

	std::vector<const char *> opts;
	Server::showBoard(this, &opts);
}

void CharacterSession::countryList()
{
	std::vector<Character *> *list = DataService::getService()->getCountryList();
	Server::sendCountryList(this, list, list->size(), 1, 99);
	delete list;
}

void CharacterSession::resumeConv(DAPacket *p)
{
	service->decrypt(p);
	unsigned char k0, k1;
	k0 = p->extractByte();
	k1 = p->extractByte();
	p->skip(4);
	unsigned short len = p->getDataLen() - 6;

	dialogDecrypt(k0, k1, len, p->getCurPtr());
	p->skip(1);
	int oid = p->extractInt();
	unsigned short opt = p->extractShort();

	LockedChar lc = character;
	Map *m = lc->getMap();
	Entity *e = m->getNearByOid(lc.get(), oid);
	if (e || !oid) {
		if (opt == 0x4A || opt == 0x11 || opt == 0x17) { //0x4a = buy, 0x11 = learn secret 0x17 = learn skill
			std::string rep = p->extractString();
			resumeDlg(lc.get(), e, rep);
		}
		else if (opt == 0x4D || opt == 0x13 || opt == 0x19) { //0x13 = forget secret 0x19 = forget skill
			unsigned short slot;
			char s = p->extractByte();
			if (opt == 0x4D) {
				slot = s - 1;
			}
			else {
				slot = s;
			}

			resumeDlg(lc.get(), e, slot);
		}
		else if (opt == 0x4F) {
			/*short check = */p->extractShort(); //not sure what check should be but its not needed
			std::string rep = p->extractString();
			resumeDlg(lc.get(), e, rep);
		}
		else
			resumeDlg(lc.get(), e, opt);
	}
	else {
		//Player made a bad request
	}
}

void CharacterSession::resumeConvLong(DAPacket *p)
{
	service->decrypt(p);
	unsigned char k0, k1;
	k0 = p->extractByte();
	k1 = p->extractByte();
	p->skip(4);
	unsigned short len = p->getDataLen() - 6;

	dialogDecrypt(k0, k1, len, p->getCurPtr());
	p->skip(1);
	int oid = p->extractInt();
	p->skip(2);
	short off = p->extractShort();

	std::string rep;
	if (p->extractByte() == 2)
		rep = p->extractString();

	LockedChar lc = character;
	Map *m = lc->getMap();
	Entity *e = m->getNearByOid(lc.get(), oid);
	if (e || !oid) {
		if (rep.empty())
			resumeDlg(lc.get(), e, off);
		else
			resumeDlg(lc.get(), e, rep);
	}
}

void CharacterSession::dropItem(DAPacket *p)
{
	service->decrypt(p);
	char slot = p->extractByte();
	unsigned short x = p->extractShort();
	unsigned short y = p->extractShort();
	unsigned int numDropped = p->extractInt();

	LockedChar lc = character;
	lc->dropItem(slot, x, y, numDropped);
}

void CharacterSession::unequipItem(DAPacket *p)
{
	service->decrypt(p);
	char slot = p->extractByte();

	LockedChar lc = character;
	lc->unequip(slot);
}

void CharacterSession::whisper(DAPacket *p)
{
	service->decrypt(p);
	std::string recip = p->extractString();
	std::string msg = p->extractString();

	if (recip == "!!")
		DataService::getService()->groupChat(this, msg);
	else if (recip == "!")
		DataService::getService()->guildChat(this, msg);
	else
		DataService::getService()->sendWhisper(this, recip, msg);
}

void CharacterSession::dropGold(DAPacket *p)
{
	service->decrypt(p);
	unsigned int amtDropped = p->extractInt();
	unsigned short x = p->extractShort();
	unsigned short y = p->extractShort();

	LockedChar lc = character;
	lc->dropGold(amtDropped, x, y);
}

void CharacterSession::movePane(DAPacket *p)
{
	service->decrypt(p);
	char pane = p->extractByte();
	char src = p->extractByte();
	char dst = p->extractByte();

	LockedChar lc = character;
	if (pane == 2) { //skills pane
		lc->swapSkills(dst,src);
	}
	else if (pane == 1) { //spells pane
		lc->swapSecrets(dst,src);
	}
	else if (pane == 0) { //inventory
		lc->swapInv(dst,src);
	}
}

void CharacterSession::attack()
{
	LockedChar lc = character;
	lc->attack();
}

void CharacterSession::refresh()
{
	LockedChar lc = character;
	lc->refresh();
}

void CharacterSession::useSkill(DAPacket *p)
{
	service->decrypt(p);
	char slot = p->extractByte();

	LockedChar lc = character;
	lc->useSkill(slot);
}

void CharacterSession::useItem(DAPacket *p)
{
	service->decrypt(p);
	char slot = p->extractByte();

	LockedChar lc = character;
	lc->useItem(slot);
}

void CharacterSession::turn(DAPacket *p)
{
	service->decrypt(p);

	char dir = p->extractByte();

	LockedChar lc = character;
	lc->tryTurn(dir); //ignoring turn failure
}

void CharacterSession::clicked(DAPacket *p)
{
	service->decrypt(p);

	char clickType = p->extractByte(); //1 = click on entity, 3 = click on ground
	if (clickType == 3) {
		unsigned short x = p->extractShort();
		unsigned short y = p->extractShort();
		/*char active = */p->extractByte(); //indicates if the player clicked from the background
		//if (active)
		LockedChar lc = character;
		lc->clickGround(x, y);
	}
	else if (clickType == 1) {
		int oid = p->extractInt();

		LockedChar lc = character;
		lc->clickEntity(oid);
	}
}

void CharacterSession::logout(DAPacket *p)
{
	Server::endSignal(this);

	//redirect to login
	using namespace config;
	char redirectData[] = {ipaddr[3], ipaddr[2], ipaddr[1], ipaddr[0]};
	DAPacket replyTwo(Server::REDIRECT, redirectData, sizeof(redirectData));

	replyTwo.appendShort(loginPort);
	replyTwo.appendByte(character->getName().length() + service->getKeyLen() + 3);
	replyTwo.appendByte(service->getSeed());
	replyTwo.appendString(service->getKeyLen(), service->getKey());
	replyTwo.appendString(character->getName().length(), character->getName().c_str());
	replyTwo.appendString(0, 0);

	lock();
	replyTwo.writeData(fd);
	unlock();

	//TODO Fix logout sequence
	//open = false;
}

void CharacterSession::move(DAPacket *p)
{
	service->decrypt(p);
	char dir = p->extractByte();
	unsigned char moveOrdinal = p->extractByte();
	if (dir > 4) { //DIR_MAX
		open = false;
		return;
	}

	//Check if moved out of order
	if (++(this->moveOrdinal) != moveOrdinal) {
		//open = false;
		//return;
	}

	//Move the player
	LockedChar lc = character;
	if (!lc->tryMove(dir)) {
		// Report failed move
		Server::movedSelf(this, lc->getX(), lc->getY(), 4);

	}

}

void CharacterSession::talk(DAPacket *p)
{
	service->decrypt(p);

	unsigned char domain = p->extractByte();
	std::string text = p->extractString();

	ChatCommand c = tryGetCommand(text.c_str());

	if (c.cmd == NONE) {
		//Normal chat, could filter now

		//Max chat length
		char display[68];
		int len;

		if ((len = snprintf(display, 68, "%s: %s", character->getName().c_str(), text.c_str())) >= 88) {
			return;
		}
		unsigned char data[] = {0, domain};
		DAPacket talk(Server::TALK, (char *)data, 2);
		talk.appendInt(character->getOid());
		talk.appendString(len, display);
		talk.appendByte('\0');
		talk.appendByte('\0');
		talk.appendByte('\0');

		if (domain == 1) {
			DataService::getService()->sendToAll(&talk, Character::Settings::LISTEN_TO_SHOUT);

			char data2[] = {0, 3, 0};
			DAPacket alert(Server::SYSTEM_MESSAGE, data2, 3);
			alert.appendString(len, display);

			DataService::getService()->sendToAll(&alert, Character::Settings::LISTEN_TO_SHOUT);
			LOG4CPLUS_INFO(core::log, display);
		}
		else if (domain == 0) {
			LockedChar lc = character;
			Map *m = lc->getMap();
			m->talked(lc.get(), text);
		}
		else {
			//Unrecognised domain
		}

	}
	//TODO Command handler
	else if (c.cmd == SET_BGM) {
		if (c.p1) {
			unsigned char data7[] = {ordinal++, 0xff, (unsigned char)c.p1, 0};
			DAPacket rep(Server::PLAY_SOUND, (char *)data7, 4);
			service->encrypt(&rep);

			lock();
			rep.writeData(fd);
			bgmPlayerSet = true;
			unlock();
		}
		else {
			lock();
			bgmPlayerSet = false;
			unlock();
		}
	}
	else if (c.cmd == GOTO_MAP) {
		if (character->getPrivilegeLevel() < 1) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			LockedChar lc = character;
			//DataService::getService()->leaveMap(character);
			if (!DataService::getService()->tryChangeMap(lc.get(), c.p1, c.p2, c.p3, 3)) {
				systemMessage("Map not found, or coordinates given were invalid", 3);
			}
		}
	}
	else if (c.cmd == SPAWN_MOB) {
		if (character->getPrivilegeLevel() < 10) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			LockedChar lc = character;
			Mob *mob = mob::MobInfo::spawnById(c.p1, lc->getX(),
					lc->getY(), lc->getMap());
			if (!mob)
				systemMessage("Mob not found.", 3);
		}
	}
	else if (c.cmd == PLAY_EFFECT) { //TODO privilege level up
		LockedChar lc = character;
		lc->playEffect(c.p1, 100);
	}
	else if (c.cmd == ITM) {
		if (character->getPrivilegeLevel() < 5) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			BaseItem *base = BaseItem::getById(c.p1);
			if (!base) {
				Server::sendMessage(this, "Item not found");
				return;
			}
			Item *item = new Item(base);
			LockedChar lc = character;
			if (!lc->getItem(item))
				delete item;
		}
	}
	else if (c.cmd == PVP) {
		if (character->getPrivilegeLevel() < 6) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			LockedChar lc = character;
			Map *m = lc->getMap();
			m->togglePvp();
		}
	}
	else if (c.cmd == RELOAD) {
		if (character->getPrivilegeLevel() < 7) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			initScripts();
		}
	}
	else if (c.cmd == INVITE) {
		DataService::getService()->groupInvite(character, c.c1);
	}
	else if (c.cmd == CHANGE_PW) {
		if (character->getPrivilegeLevel() < 10) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			char name[13];
			strncpy(name, c.c1, c.p3);
			name[c.p3] = 0;
			Database::setPassword(name, c.c2);
		}
	}
	else if (c.cmd == IP_BAN) {
		if (character->getPrivilegeLevel() < 10) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			try {
				DataService::getService()->blacklist(c.c1, character->getPrivilegeLevel());
			}
			catch (SessionError &se) {
				switch (se) {
				case E_INVALID:
					systemMessage("You don't have a high enough privilege level to ban that user.", 3);
					break;
				case E_NOEXIST:
					systemMessage("No user with that name is currently logged in.", 3);
					break;
				default:
					systemMessage("Something went wrong", 3);
					log4cplus::Logger log = log4cplus::Logger::getInstance("renaissance.server");
					LOG4CPLUS_WARN(log, "An unexpected exception occurred during an IP ban request.");
					break;
				}
			}
		}
	}
	else if (c.cmd == CHAR_BAN) {
		if (character->getPrivilegeLevel() < 10) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			try {
				DataService::getService()->banChar(c.c1, character->getPrivilegeLevel());
			}
			catch (SessionError &se) {
				switch (se) {
				case E_INVALID:
					systemMessage("You don't have a high enough privilege level to ban that user.", 3);
					break;
				case E_NOEXIST:
					systemMessage("No user with that name is currently logged in.", 3);
					break;
				default:
					systemMessage("Something went wrong", 3);
					log4cplus::Logger log = log4cplus::Logger::getInstance("renaissance.server");
					LOG4CPLUS_WARN(log, "An unexpected exception occurred during a character block request.");
					break;
				}
			}
		}
	}
	else if (c.cmd == GOTO_PLAYER) {
		if (character->getPrivilegeLevel() < 8) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			try {
				DataService::getService()->gotoChar(c.c1, character);
			}
			catch (SessionError &se) {
				switch (se) {
				case E_NOEXIST:
					systemMessage("No user with that name is currently logged in.", 3);
					break;
				case E_UNSPECIFIED:
				default:
					systemMessage("Something went wrong.", 3);
					break;
				}
			}
		}
	}
	else if (c.cmd == RECALL_PLAYER) {
		if (character->getPrivilegeLevel() < 8) {
			systemMessage("Check your privileges.", 3);
		}
		else {
			try {
				DataService::getService()->recallChar(c.c1, character->getPrivilegeLevel(), character);
			}
			catch (SessionError &se) {
				switch (se) {
				case E_INVALID:
					systemMessage("You don't have a high enough privilege level to recall that user.", 3);
					break;
				case E_NOEXIST:
					systemMessage("No user with that name is currently logged in.", 3);
					break;
				case E_UNSPECIFIED:
				default:
					systemMessage("Something went wrong.", 3);
					break;
				}
			}
		}
	}
}

void CharacterSession::systemMessage(const char *msg, unsigned char channel)
{
	lock();
	unsigned char data[3] = {ordinal++, channel, 0};
	DAPacket sysMsg(Server::SYSTEM_MESSAGE, (char *)data, 3);
	sysMsg.appendString(strlen(msg), msg);
	service->encrypt(&sysMsg);
	sysMsg.writeData(fd);
	unlock();
}

void CharacterSession::pickupItem(DAPacket *p)
{
	service->decrypt(p);
	unsigned char slot = p->extractByte();
	unsigned short x = p->extractShort();
	unsigned short y = p->extractShort();

	LockedChar lc = character;
	lc->pickupItem(slot, x, y);
}

void CharacterSession::getExtraInfo()
{
    // TODO not sure if there is a response to this message
}

void CharacterSession::getLegendData()
{
	DAPacket *rep;
	unsigned char data[] = {ordinal++};
	rep = new DAPacket(Server::LEGEND_DATA, (char *)data, 1);

	{
		LockedChar lc = character;
		lc->getLegendInfo(rep);
	}

	service->encrypt(rep);

	lock();
	rep->writeData(fd);
	delete rep;
	unlock();
}

void CharacterSession::attachCharacter(DAPacket *p)
{
	if (0 != character) {
		//not allowed to attach more than once
		open = false;
		return;
	}
	unsigned char seed = p->extractByte();
	//TODO fix table 9
	if (seed >= 9) {
		open = false;
		return;
	}

	std::string key(p->extractString());
	std::string name(p->extractString());

	if (name == "" || key.length() != 9) {
		open = false;
		return;
	}
	service = new EncryptionService(name.c_str(), key.c_str(), key.length(), seed);
	character = DataService::getService()->getCharacter(name, this, clientIp);
	if (0 == character) {
		delete service;
		service = 0;
		open = false;
		return;
	}

	getCharacterInfo();
}

void CharacterSession::incStat(DAPacket *p)
{
	service->decrypt(p);
	char which = p->extractByte();
	//str=1, wis=0x10, int=0x04, dex=0x02 presumably con=0x08, oddly not synch with stat order
	//server response is 0x0A (message) then 0x08 0x24 (updated primary + secondary, no hpmp or points)

	LockedChar lc = character;
	lc->incStat(which);
}

void CharacterSession::getCharacterInfo()
{
	DAPacket *rep;
	Server::updateStatInfo(this, character, Character::FLAG_ALL);

	//not sure what this one does, changing seems to have no effect
	unsigned char data2[] = {ordinal++, 6, 0, 0};
	rep = new DAPacket(Server::UNK_1, (char *)data2, 4);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	unsigned char data3[] = {ordinal++, 8, 1}; //sets time of day to afternoon?
	rep = new DAPacket(Server::SET_LIGHTING, (char *)data3, 3);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	systemMessage(config::motd, 3);

	char buffer[128];
	gameDatetime(buffer, 128);
	systemMessage(buffer, 3);

	unsigned char data5[] = {ordinal++}; //, 0, 2, 0x61, 0x04, 2, 0, 0, 0, 0, 0};
	rep = new DAPacket(Server::SET_OID, (char *)data5, 1);
	rep->appendInt(character->getOid());
	rep->appendShort(0x200); //think this is 100 for male, doesn't seem to matter
	rep->appendByte(character->getPath());//client probably only uses for the tab map
	rep->appendByte(0);
	rep->appendByte(*character->getGender() == 'f' ? 1 : 0);//just a guess
	rep->appendByte(0);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	{
		LockedChar lc = character;
		DataService::getService()->enterMap(character);
	}

	lock();

	unsigned char data10[] = {ordinal++, 0, 0};
	rep = new DAPacket(Server::UNK_11, (char *)data10, 3);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	unsigned char data14[] = {ordinal++, 0};
	rep = new DAPacket(Server::UNK_9, (char *)data14, 2);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	//not sure if this packet is required
	unsigned char data15[] = {ordinal++, 0xa, 0};
	rep = new DAPacket(Server::NPC_DIALOG, (char *)data15, 3);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	/*char data16[] = {ordinal++, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0x63, 0, 0, 0};
	rep = new DAPacket(Server::CHAR_DATA, data16, sizeof(data16)-1);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	data16[0] = ordinal++;
	rep = new DAPacket(Server::CHAR_DATA, data16, sizeof(data16)-1);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;

	data16[0] = ordinal++;
	rep = new DAPacket(Server::CHAR_DATA, data16, sizeof(data16)-1);
	service->encrypt(rep);
	rep->writeData(fd);
	delete rep;*/
	//probably unnecessary

	unlock();

	unsigned char metapath = 0x30;
	//TODO does this change for masters?
	metapath += character->getPath();
	Server::sendMetadata(this, metapath);

}
