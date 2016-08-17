/*
 * CharBindings.h
 *
 *  Created on: 2013-08-29
 *      Author: per
 */

#ifndef CHARBINDINGS_H_
#define CHARBINDINGS_H_

#include "lua.hpp"

namespace script {
	void loadCharBindings(lua_State *L);

	//Basic stat getters
	int baseHp(lua_State *L);
	int baseMp(lua_State *L);
	int curWeight(lua_State *L);
	int getPath(lua_State *L);
	int getGender(lua_State *L);
	int countLabor(lua_State *L);
	int getEquip(lua_State *L);
	int countItems(lua_State *L);
	int countKills(lua_State *L);
	int countItemId(lua_State *L);

	//Basic state booleans
	int isMaster(lua_State *L);
	int isDead(lua_State *L);
	int setMaster(lua_State *L);
	int isPure(lua_State *L);

	//Quest functions
	int getQuestProgress(lua_State *L);
	int getQuestTimer(lua_State *L);
	int addTracker(lua_State *L);
	int delTracker(lua_State *L);
	int setQuestProgress(lua_State *L);
	int setQuestTimer(lua_State *L);
	int addLegendMark(lua_State *L);
	int updateLegendQty(lua_State *L);
	int getLegendText(lua_State *L);
	int getLegendTimestamp(lua_State *L);
	int removeLegend(lua_State *L);
	int getSkillLevel(lua_State *L);

	//Basic stat modifiers
	int addMaxHp(lua_State *L);
	int addMaxMp(lua_State *L);
	int incStat(lua_State *L);
	int forgetSkill(lua_State *L);
	int setPath(lua_State *L);
	int setHairstyle(lua_State *L);
	int setHairColor(lua_State *L);
	int giveExp(lua_State *L);
	int takeExp(lua_State *L);
	int revive(lua_State *L);
	int teleport(lua_State *L);
	int chargeLabor(lua_State *L);
	int changeAttr(lua_State *L);

	//Basic skill functions
	int giveSkill(lua_State *L);
	int getSkillSublist(lua_State *L);
	int removeSkill(lua_State *L);

	//Advanced functions
	int rededicate(lua_State *L);
	int createGuild(lua_State *L);
	int inGuild(lua_State *L);
	int addMember(lua_State *L);
	int removeMember(lua_State *L);
	int promoteMember(lua_State *L);
	int listMembers(lua_State *L);
	int deleteGuild(lua_State *L);

	//Inventory functions
	int maxStack(lua_State *L);
	int getItemName(lua_State *L);
	int repair(lua_State *L);
	int sellItem(lua_State *L);
	int getSaleValue(lua_State *L);
	int buyItem(lua_State *L);
	int canSellItem(lua_State *L);
	int charge(lua_State *L);
	int giveItem(lua_State *L);
	int getRepairCost(lua_State *L);
	int repairAll(lua_State *L);
	int withdrawItem(lua_State *L);
	int canDeposit(lua_State *L);
	int chargeGold(lua_State *L);
	int depositItem(lua_State *L);
	int storageInfo(lua_State *L);
	int depositGold(lua_State *L);
	int withdrawGold(lua_State *L);
	int countStoredGold(lua_State *L);
	int getItemId(lua_State *L);

}


#endif /* CHARBINDINGS_H_ */
