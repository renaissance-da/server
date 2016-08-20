/*
 * GameEngine.cpp
 *
 *  Created on: 2012-12-13
 *      Author: per
 */

#include "GameEngine.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <time.h>
#include "random.h"
#include "BaseItem.h"
#include "SkillInfo.h"
#include "defines.h"
#include "ai_broker.h"
#include "StandardAI.h"
#include "LuaAI.h"
#include "core.h"
#include <chrono>
#include <thread>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

void registerAI()
{
    mob::registerAI(0, "Standard AI", [=](int param, Mob *m) {
        return new StandardAI(m, param);
    });
    mob::registerAI(1, "Scripted AI", [=](int which, Mob *m) {
        return new LuaAI(which, m);
    });
}

GameEngine::GameEngine(DataService *s, drand48_data *rng) :
    service(s), rng(rng)
{
    running = true;
    registerAI();
	thread = std::thread(gameLoop, this);
}

GameEngine::~GameEngine()
{
    if (running) {
        running = false;
		thread.join();
    }
    service->clearAll();

    BaseItem::clear();
    SkillInfo::clear();
}

void GameEngine::gameLoop(GameEngine *engine)
{

    rngInit(engine->rng);

	auto sleep_interval = std::chrono::milliseconds(1000 / TICKS);

    while (engine->running) {
        //update all maps every sec

		auto t0 = std::chrono::high_resolution_clock::now();
        engine->service->tick();
		auto t1 = std::chrono::high_resolution_clock::now();
		auto delta =
			std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

		if (delta > std::chrono::milliseconds(1000000 / TICKS)) {

			LOG4CPLUS_WARN(core::log(),
				"Game engine is behind! Loop took " << delta.count() <<
				" microseconds.");
        }
        else {
			std::this_thread::sleep_for(sleep_interval - delta);
        }
    }
}

bool GameEngine::isRunning()
{
    return running;
}
