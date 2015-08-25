/*
 * srv_proto.h
 *
 *  Created on: 2012-12-17
 *      Author: per
 */

#ifndef SRV_PROTO_H_
#define SRV_PROTO_H_

#include "Entity.h"
#include "Door.h"
#include "Skill.h"
#include "Character.h"
#include "Field.h"
#include <vector>
#include "Secret.h"
class CharacterSession;

// This is a collection of functions that generate outbound messages
// TODO This should be a class, eg OutCharacterSession
namespace Server
{
void npcDlg(CharacterSession *s, Entity *e, const char *msg,
	    unsigned short msgLen, char opts, const char **optList);
void leaveMap(CharacterSession *s);
void enterMap(CharacterSession *s, unsigned short mapId, char width,
	      char height, unsigned short checksum, const char *name,
	      char nameLen);
void entGone(CharacterSession *s, unsigned int oid);
void showViewable(CharacterSession *s, Viewable *v);
void entActed(CharacterSession *s, unsigned int oid, char anim, short animLen);
void playSound(CharacterSession *s, char sound);
void setBgm(CharacterSession *s, char bgm);
void setCoords(CharacterSession *s, unsigned short x, unsigned short y);
void showExtended(CharacterSession *s, Viewable *v);
void sendDoors(CharacterSession *s, const std::vector<Door *> &doors);
void showDoor(CharacterSession *s, Door *d);
void send0x58(CharacterSession *s);
void entTurned(CharacterSession *s, Entity *e);
void entMoved(CharacterSession *s, Entity *e, char dir);
void showViewables(CharacterSession *s, const std::vector<Viewable *> &views);
void startCd(CharacterSession *s, char pane, char slot, unsigned int cd);
void eraseSkill(CharacterSession *s, char slot);
void eraseSecret(CharacterSession *s, char slot);
void getSkill(CharacterSession *s, Skill *sk, char slot);
void movedSelf(CharacterSession *s, unsigned short x, unsigned short y,
	       char dir);
void sendMessage(CharacterSession *s, const char *msg,
		 unsigned char channel = 3);
void entStruck(CharacterSession *s, unsigned int oid, unsigned short hp,
	       unsigned char sound);
void updateStatInfo(CharacterSession *s, Character *c, unsigned int flags);
void getItem(CharacterSession *s, unsigned char slot, unsigned char qty,
	     const char *name, unsigned short nameLen, unsigned short apr,
	     unsigned int dur, unsigned int maxDur);
void equipItem(CharacterSession *s, unsigned char slot, const char *name,
	       unsigned short nameLen, unsigned short apr, unsigned int dur,
	       unsigned int maxDur);
void playEffect(CharacterSession *s, unsigned int oid, unsigned short anim,
		unsigned int animLen);
void playEffect(CharacterSession *s, unsigned short x, unsigned short y,
		unsigned short anim, unsigned short animLen);
void removeItem(CharacterSession *s, unsigned char slot);
void removeEquip(CharacterSession *s, unsigned char slot);
void sendVendList(CharacterSession *s, Entity *e, short textLen,
		  const char *text, std::vector<BaseItem *> &items);
void sendVendList(CharacterSession *s, Entity *e, short textLen,
		  const char *text,
		  const std::vector<Character::StorageItem> &items);
void sendItemSublist(CharacterSession *s, Entity *e, short textLen,
		     const char *text, std::vector<char> &slots);
void sendCountryList(CharacterSession *s, std::vector<Character *> *list,
		     short total, short lmin, short lmax);
void showBoard(CharacterSession *s, std::vector<const char *> *opts);
void sendSkillList(CharacterSession *s, Entity *e, short textLen,
		   const char *text, std::vector<SkillInfo *> &skills,
		   bool spellList = false);
void fieldWarp(CharacterSession *s, const char *name,
	       std::vector<Field::FieldDest> &fieldDests);
void sendMetadata(CharacterSession *s, unsigned char metapath);
void talked(CharacterSession *s, int oid, char channel, const char *text,
	    int textLen);
void getSecret(CharacterSession *s, Secret *sc, char slot, int weapon);
void profileInfo(CharacterSession *s, Character *c);
void groupInvite(CharacterSession *s, unsigned char nameLen, const char *name);
void sendStatusEffect(CharacterSession *s, unsigned short effect,
		      unsigned char dur);
void closeDialog(CharacterSession *s);
void sendLongDlg(CharacterSession *s, Entity *e, const char *msg,
		 unsigned short msgLen, bool next, bool prev, char pos,
		 char opts, const char **optList);
void confirmExchange(CharacterSession *s, bool local);
void cancelExchange(CharacterSession *s, bool local);
void exchangeAddItem(CharacterSession *s, bool local, Item *itm, char pos);
void exchangeAddGold(CharacterSession *s, bool local, int amt);
void startExchangeWith(CharacterSession *s, int oid, const char *name);
void initExchange(CharacterSession *s, int oid, unsigned char slot);
void initGoldExchange(CharacterSession *s, int oid, unsigned int amt);
void sendFile(CharacterSession *s, const unsigned char *file, unsigned char w,
	      unsigned char h);
void endSignal(CharacterSession *s);
void queryAmount(CharacterSession *s, Entity *e, short textLen,
		 const char *text);
void queryCustom(CharacterSession *s, Entity *e, short int textLen,
		 const char *text, short int queryLen, const char *queryBox);
void exchangeHowMany(CharacterSession *s, char slot);
void sendStayAlive(CharacterSession *s);
void sendForgetList(CharacterSession *s, Entity *e, short textLen,
		    const char *text, bool secrets);
void updateSettings(CharacterSession *s, int setting);
}

#endif /* SRV_PROTO_H_ */
