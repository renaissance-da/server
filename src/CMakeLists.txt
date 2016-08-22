cmake_minimum_required (VERSION 2.6)
project (renaissance)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(UNIX)
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DTHREADSAFE -D_REENTRANT")
endif()

set(PostgreSQL_ADDITIONAL_VERSIONS "9.3.4")
set(PostgreSQL_ADDITIONAL_SEARCH_PATHS "/usr/include/postgresql/9.3/server")
set(PostgreSQL_INCLUDE_DIR "/usr/include/postgresql")
set(PostgreSQL_LIBRARY_DIR "/usr/lib/postgresql")
find_package(PostgreSQL REQUIRED)

find_package(Lua REQUIRED)
set( CMAKE_THREAD_PREFER_PTHREAD )
find_package(Threads REQUIRED)
find_package(Log4cplus REQUIRED)
# find_library(LIBCONFIG_LIBRARY NAMES libconfig++)
# set(LIBCONFIG_LIBRARIES ${LIBCONFIG_LIBRARY})

set (renaissance_SRCS DataService.cpp main.cpp char/BaseConsumable.cpp    core/Stats.cpp
char/BaseEquipment.cpp   core/core.cpp           core/StatusEffect.cpp
char/BaseItem.cpp        common/md5hash.cpp      core/StatusTrap.cpp
char/Character.cpp       common/network.cpp      core/Trap.cpp
char/CharacterList.cpp   common/random_engines.cpp       core/Trigger.cpp
char/Consumable.cpp      core/BaseEffect.cpp     Database/CharManager.cpp
char/Equipment.cpp       core/Combat.cpp         Database/DataLoaders.cpp
char/Exchange.cpp        core/Door.cpp           mob/MobAI.cpp
char/GameTime.cpp        core/element.cpp        mob/Mob.cpp
char/GroundItem.cpp      core/Entity.cpp         mob/Spawner.cpp
char/Inventory.cpp       core/Field.cpp          mob/StandardAI.cpp
char/Item.cpp            core/GameEngine.cpp     network/CharacterServer.cpp
char/Legend.cpp          core/Map.cpp            network/CharacterSession.cpp
char/Parser.cpp          core/MapPoint.cpp       network/DAPacket.cpp
char/Paths.cpp           core/NPC.cpp            network/EncryptionService.cpp
char/Tracker.cpp         core/Portal.cpp         network/LoginServer.cpp
common/BasicServer.cpp   core/ScriptTrigger.cpp  network/LoginSession.cpp
common/BasicSession.cpp  core/Secret.cpp         network/srv_proto.cpp
common/config.cpp        core/SecretInfo.cpp     script/char_bindings.cpp
common/crc.cpp           core/Skill.cpp          script/dialogHandlers.cpp
common/Hash.cpp          core/SkillInfo.cpp      script/npc_bindings.cpp
core/Viewable.cpp        mob/MobWatcher.cpp      mob/MobInfo.cpp
mob/LuaAI.cpp            script/ai_bindings.cpp  script/entity_bindings.cpp
script/map_bindings.cpp  script/script.cpp       mob/ai_broker.cpp
core/Timer.cpp           script/ScriptTimer.cpp  char/Group.cpp
core/Instance.cpp        char/Guild.cpp          common/LockSet.cpp
char/LockedChar.cpp)

include_directories(. char common core Database mob network script ${PostgreSQL_INCLUDE_DIRS} ${LUA_INCLUDE_DIR}
  ${LOG4CPLUS_INCLUDE_DIR})

add_executable(DAServer ${renaissance_SRCS})

target_link_libraries ( DAServer ${PostgreSQL_LIBRARIES} ${LUA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} "-lconfig++"
  ${LOG4CPLUS_LIBRARIES} "-lz" )
