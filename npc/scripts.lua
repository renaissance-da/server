math.randomseed( os.time() )
math.random()

function npc_query(text, opts)
   NPC_sendMessage(c_char, c_npc, text, table.unpack(opts))
   opt = coroutine.yield() 
   if type(opt) == type('') then
      return
   else if opt > #opts or opt < 1 then
	 return
      else
	 return opt
      end
   end
end

function countItemId(itemId, mod)
   if mod then
      return NPC_countItemId(c_char, itemId, mod)
   else
      return NPC_countItemId(c_char, itemId)
   end
end

function getEquip(slot)
   return NPC_getEquip(c_char, slot)
end

function getLevel()
   return NPC_getLevel(c_char)
end

function addMaxHp(amt)
   NPC_addMaxHp(c_char, amt)
end

function incStr()
   return NPC_incStat(c_char, 1)
end

function incDex()
   return NPC_incStat(c_char, 2)
end

function incInt()
   return NPC_incStat(c_char, 3)
end

function incWis()
   return NPC_incStat(c_char, 4)
end

function incCon()
   return NPC_incStat(c_char, 5)
end

function curWeight()
   return NPC_curWeight(c_char)
end

function addMaxMp(amt)
   NPC_addMaxMp(c_char, amt)
end

function rededicate(p)
   NPC_rededicate(c_char, p)
end

-- returns true if the dialog finished normally, false if closed by user
function npc_longdlg(texts)
   local off = 1
   while (texts[off]) do
      NPC_longDialog(c_char, c_npc, texts[off], off, off > 1, off <= # texts)
      newoff = coroutine.yield()
      if (newoff == off) then
	 return false
      end
      off = newoff
   end
   return true
end

function questProgress(questId)
   return NPC_getQuestProgress(c_char, questId)
end

function restoreMp(amt)
   return NPC_restoreMp(c_char, amt)
end

function setProgress(questId, progress)
   return NPC_setQuestProgress(c_char, questId, progress)
end

function questTimer(questId)
   return NPC_getQuestTimer(c_char, questId)
end

function setTimer(questId, timer)
   return NPC_setQuestTimer(c_char, questId, timer)
end

function getTimer(questId)
   return NPC_getQuestTimer(c_char, questId)
end

function giveExp(amt)
   return NPC_giveExp(c_char, amt)
end

function takeExp(amt)
   return NPC_takeExp(c_char, amt)
end

function isMaster()
   return NPC_isMaster(c_char)
end

function setMaster()
   return NPC_setMaster(c_char)
end

function teleport(map, x, y)
   return NPC_teleport(c_char, map, x, y)
end

function setHairstyle(hairstyle)
   return NPC_setHairstyle(c_char, hairstyle)
end

function endDialog()
   NPC_closeDialog(c_char)
end

function teleportInstance(mapid, x, y, initFn)
   local instId = NPC_teleportInstance(c_char, mapid, x, y)
   if instId and initFn then
      initFn(instId)
   end
end

function chooseRepair(msg)
   local slot = NPC_showRepairList(c_char, c_npc, msg)
   return coroutine.yield()
end

function addLegendMark(markid, textparam, intparam, time)
   if time == nil then
      NPC_addLegendMark(c_char, markid, textparam, intparam)
   else
      NPC_addLegendMark(c_char, markid, textparam, intparam, time)
   end
end

function removeLegend(markid)
   NPC_removeLegend(c_char, markid)
end

function updateLegendQty(markid, intparam, textParam)
   if textParam then
      NPC_updateLegendQty(c_char, markid, intparam, textParam)
   else
      NPC_updateLegendQty(c_char, markid, intparam)
   end
end

function countItem(slot)
   return NPC_countItems(c_char, slot)
end

function maxStack(item)
   return NPC_maxStack(c_char, item)
end

function queryAmount(text)
   NPC_queryText(c_char, c_npc, text)
   local qty = coroutine.yield()
   return tonumber(qty)
end

function queryText(text)
   NPC_queryText(c_char, c_npc, text)
   return coroutine.yield()
end

function npc_message(text)
   NPC_sendMessage(c_char, c_npc, text)
   --coroutine.yield()
end

function npc_vend(text, items)
   NPC_sendVendList(c_char, c_npc, text, table.unpack(items))
   local opt = coroutine.yield()
   return opt
end

NPCS = {}
cos = {}
c_npc = nil
c_char = nil
c_id = nil
Scripts = {}
Spells = {}
Effects = {}
Timers = {}

function addNpc(map, x, y, name, apr, dir, begin)
   local id = NPC_addnpc(map, x, y, name, apr, dir)
   NPCS[id] = {begin}
   return id
end

function addTimer(map, delay, fn)
   tid = #Timers
   Timers[tid] = coroutine.create(fn)
   NPC_addTimer(map, delay, tid)
end

function addTrigger(map, x, y, begin, lifetime, dropped)
   if not lifetime then
      lifetime = -1
   end
   local id = NPC_addtrigger(map, x, y, lifetime)
   Scripts[id] = {begin, dropped}
   return id
end

function countLabor()
   return NPC_countLabor(c_char)
end

function charge(items)
   return NPC_charge(c_char, table.unpack (items))
end

function chargeLabor(amt)
   return NPC_chargeLabor(c_char, amt)
end

function giveItem(id, amt, mod)
   if mod then
      return NPC_giveItem(c_char, id, amt, mod)
   else
      return NPC_giveItem(c_char, id, amt)
   end
end

function getPath()
   return NPC_getPath(c_char)
end

function isPure()
   return NPC_isPure(c_char)
end

function sysMsg(msg)
   return NPC_sysmsg(c_char, msg)
end

function playEffect(effect)
   return NPC_playEffect(c_char, effect)
end

function playSound(sound)
   return NPC_playSound(c_char, sound)
end

function drainHp(hp)
   return NPC_drainHp(c_char, hp)
end

function drainMp(mp)
   return NPC_restoreMp(c_char, -mp)
end

function setPath(p)
   NPC_setPath(c_char, p)
end

function getGender()
   return NPC_getGender(c_char)
end

function itemOnList(itm, list)
   return NPC_itemOnList(itm, table.unpack(list))
end

function sellItem(itm, amt)
   return NPC_sellItem(c_char, itm, amt)
end

function sellList(text, items)
   if items then
      NPC_showSellList(c_char, c_npc, text, table.unpack(items))
   else
      NPC_showSellList(c_char, c_npc, text)
   end
   return coroutine.yield()
end

function getBaseHp()
   return NPC_baseHp(c_char)
end

function getBaseMp()
   return NPC_baseMp(c_char)
end

function changeAttr(s, d, c, i, w)
   NPC_changeAttr(c_char, s, d, c, i, w)
end

function removeSkill(skill)
   return NPC_removeSkill(c_char, skill)
end

function getSkillLevel(skill)
   return NPC_getSkillLevel(c_char, skill)
end

function removeSkills(skill)
   for _,v in ipairs(skill) do
      removeSkill(v)
   end
end

function offerSkills(text, isSpells, skills)
   NPC_offerSkills(c_char, c_npc, text, isSpells, table.unpack(skills))
   return coroutine.yield()
end

function giveSkill(skill, isSpell, isFree)
   if isFree then
      return NPC_giveSkill(c_char, skill, isSpell, true)
   else
      return NPC_giveSkill(c_char, skill, isSpell)
   end
end

function addTracker(trackerId, mobList)
   return NPC_addTracker(c_char, trackerId, table.unpack(mobList))
end

function delTracker(trackerId)
   return NPC_delTracker(c_char, trackerId)
end

function countKills(trackerId, mobId)
   if mobId then
      return NPC_countKills(c_char, trackerId, mobId)
   else
      return NPC_countKills(c_char, trackerId)
   end
end

-- 1 = yes, 2 = no
function skillCost(skill)
   NPC_skillCost(c_char, c_npc, skill)
   return coroutine.yield()
end

function contains(list, item)
   for _,value in pairs do
      if item == list then
	 return true
      end
   end
   return false
end

function getSaleValue(slot, amt)
   return NPC_getSaleValue(c_char, slot, amt)
end

function buyItem(slot, amt)
   NPC_buyItem(c_char, slot, amt)
end

function canSellItem(slot, list)
   return NPC_canSellItem(c_char, slot, table.unpack(list))
end

function canDeposit(slot)
   return NPC_canDeposit(c_char, slot)
end

function chargeGold(amt)
   return NPC_chargeGold(c_char, amt)
end

function createGuild(gname)
   return NPC_createGuild(c_char, gname)
end

function inGuild()
   return NPC_inGuild(c_char)
end

function initNpcsDir()
   local f = io.popen([[dir /B npc\helpers\*.lua]])
   for mod in f:lines() do
      dofile( [[npc\towns\]] .. mod )
   end

   local f = io.popen([[dir /B npc\towns\*.lua]])
   for mod in f:lines() do
      dofile( [[npc\towns\]] .. mod )
   end

   local f = io.popen([[dir /B npc\hunting_areas\*.lua]])
   for mod in f:lines() do
      dofile( [[npc\hunting_areas\]] .. mod )
   end
   
   local f = io.popen([[dir /B npc\spells\*.lua]])
   for mod in f:lines() do
      dofile( [[npc\spells\]] .. mod )
   end
   
   dofile([[npc\custom.lua]])
   loadCustoms()
end


function initNpcs()
   local f = io.popen('ls npc/helpers/*.lua')
   for mod in f:lines() do
      dofile( mod )
   end

   local f = io.popen('ls npc/towns/*.lua')

   for mod in f:lines() do
      dofile( mod )
   end

   local f = io.popen('ls npc/hunting_areas/*.lua')
   for mod in f:lines() do
      dofile( mod )
   end

   local f = io.popen('ls npc/spells/*.lua')
   for mod in f:lines() do
      dofile( mod )
   end

   local f = io.popen('ls npc/trades/*.lua')
   for mod in f:lines() do
      dofile( mod )
   end
   
   dofile("npc/custom.lua")
   loadCustoms()
end   

function initDlg(id, char, npc)
   c_id = id
   nid = NPC_getId(npc)
   if not nid then
      return
   end
   cos[id] = {}
   cos[id][1] = coroutine.create(NPCS[nid][1])
   cos[id][2] = nid
   c_npc = npc
   c_char = char
   coroutine.resume(cos[id][1])
end

function talkResponse(id, char, npc, text)
   c_id = id
   nid = NPC_getId(npc)
   if not nid then
      return
   end
   if NPCS[nid][2] then
      cos[id] = {}
      cos[id][1] = coroutine.create(NPCS[nid][2])
      cos[id][2] = nid
      c_npc = npc
      c_char = char
      coroutine.resume(cos[id][1], text)
   end
end

function talk(text)
   NPC_talk(c_npc, text)
end

function registerTalkResponse(npcid, fn)
   if NPCS[npcid] then
      NPCS[npcid][2] = fn
   end --TODO pass warning?
end

function initScript(id, char, trigger)
   c_id = id
   tid = NPC_getTriggerId(trigger)
   if not tid or not Scripts[tid] then
      return
   end
   cos[id] = {}
   cos[id][1] = coroutine.create(Scripts[tid][1])
   cos[id][2] = NPC_getId(trigger)
   c_npc = trigger
   --c_npc = nil
   c_char = char
   a, b = coroutine.resume(cos[id][1])
   if a then
      return b
   else
      print('Error in call to initScript: ' .. b)
      return true -- remove this
   end
end

function runTimer(timerId)
   if Timers[timerId] then
      a, b = coroutine.resume(Timers[timerId])
      if a then
	 return b
      else
	 print('Error in call to runTimer: ' .. b)
	 return 0
      end
   end
end

function initScriptDrop(id, char, trigger, itemId)
   c_id = id
   tid = NPC_getTriggerId(trigger)
   if not tid or not Scripts[tid] or not Scripts[tid][2] then
      return
   end
   cos[id] = {}
   cos[id][1] = coroutine.create(Scripts[tid][2])
   cos[id][2] = NPC_getId(trigger)
   c_npc = trigger
   c_char = char
   a, b = coroutine.resume(cos[id][1], itemId)
   if a then
      return b
   else
      print('Error in call to initScriptDrop: ' .. b)
      return true
   end
end

function runEffect(id, char, effectId)
   if not Effects[effectId] then
      print("Effect " .. effectId .. " not found in scripts.lua\n")
      return true
   else
      c_char = char
      return Effects[effectId]()
   end
end

function doSpell(id, char, spellId)
   if not Spells[spellId] then
      print("Spell id " .. spellId .. " not found in scripts.\n")
      return false
   else
      c_char = char
      return Spells[spellId]()
   end
end

function getPosition()
   return NPC_getPosition(c_char)
end

function resumeDlg(id, char, npc, opt)
   c_id = id
   if (cos[id][1]) then
      if (cos[id][2] == NPC_getId(npc)) then
         c_npc = npc
         c_char = char
         coroutine.resume(cos[id][1], opt)
      end
   end
end

function getItemId(slot)
   return NPC_getItemId(c_char, slot)
end

function getEquipSlot(iid)
   return NPC_getEquipSlot(iid)
end

function canCustomize(iid)
   return NPC_canCustomize(iid)
end

function switchToOther(name, noEndDlg)
   local other, otherId = NPC_getNearbyPlayer(c_char, name)
   if other then
      if not noEndDlg then
	 endDialog()
      end
      local oldc = c_id
      c_char = other
      cos[otherId] = cos[c_id]
      c_id = otherId
      return oldc
   end
end

function addMember(newc)
   return NPC_addMember(c_char, newc)
end

function removeMember(name)
   return NPC_removeMember(c_char, name)
end

function promoteCouncil(name)
   return NPC_promoteMember(c_char, name)
end

function listMembers()
   NPC_listMembers(c_char)
end

function deleteGuild()
   return NPC_deleteGuild(c_char)
end

function switchBack(oldc)
   local cnew = NPC_getNearByOid(c_char, oldc)
   if cnew then
      endDialog()
      c_char = cnew
      return true
   else
      return false
   end
end

function getName()
   return NPC_getName(c_char)
end

function recallCo(mapid, x, y)
   return function()
	     local opt = npc_query("A group member has summoned you. Will you accept?", {"Yes", "No"})
	     endDialog()
	     if opt == 1 then
		teleport(mapid, x, y)
	     end
	  end
end

function recall(id, char, mapid, x, y)
   c_char = char
   c_npc = nil
   cos[id] = {}
   cos[id][1] = coroutine.create(recallCo(mapid, x, y))
   cos[id][2] = nil
   coroutine.resume(cos[id][1])
end
