/*
 * npc_bindings.h
 *
 *  Created on: 2012-12-17
 *      Author: per
 */

#ifndef NPC_BINDINGS_H_
#define NPC_BINDINGS_H_

#ifdef WIN32
#include <lua.hpp>
#else
#include <lua5.2/lua.hpp>
#endif

#include "srv_proto.h"
#include "CharacterSession.h"
#include "NPC.h"
#include "ScriptTrigger.h"
#include "script.h"

void initScripts();

void initDlg(Character *c, NPC *npc);
void talkResponse(Character *c, NPC *npc, std::string &text);
void resumeDlg(Character *c, Entity *e, unsigned short opt);
void resumeDlg(Character *c, Entity *e, std::string &rep);
bool initScript(Character *c, ScriptTrigger *script);
bool initScript(Character *c, ScriptTrigger *script, Item *itm);
bool runEffect(Character *c, int effectId);
bool doSpell(Character *c, int spellId);
int scriptTimer(int timerId);

namespace script {

void recall(Character *c, int mapid, short x, short y);
int forgetSkills(lua_State *L);
int sendMessage(lua_State *L);
int showRepairList(lua_State *L);
int getTriggerId(lua_State *L);
int addnpc(lua_State *L);
int sendVendList(lua_State *L);
int itemOnList(lua_State *L);
int showSellList(lua_State *L);
int offerSkills(lua_State *L);
int skillCost(lua_State *L);
int closeDialog(lua_State *L);
int longDialog(lua_State *L);
int upgrade(lua_State *L);
int addtrigger(lua_State *L);
int queryText(lua_State *L);
int showStorage(lua_State *L);
int getNearbyPlayer(lua_State *L);
int getNearByOid(lua_State *L);
int getEquipSlot(lua_State *L);
int canCustomize(lua_State *L);
int sysmsg(lua_State *L);
int queryTextCustom(lua_State *L);
int spawnNewMob(lua_State *L);
int broadcast(lua_State *L);
int teleportInstance(lua_State *L);

} // namespace script
#endif /* NPC_BINDINGS_H_ */
