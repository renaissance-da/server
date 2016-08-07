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

    pthread_create(&thread, NULL, gameLoop, (void *) this);
}

GameEngine::~GameEngine()
{
    if (running) {
        running = false;
        pthread_join(thread, NULL);
    }
    service->clearAll();

    BaseItem::clear();
    SkillInfo::clear();
}

void *GameEngine::gameLoop(void *ge)
{
    GameEngine *engine = (GameEngine *) ge;

    rngInit(engine->rng);

    while (engine->running) {
        //update all maps every sec
#ifdef WIN32
        LARGE_INTEGER f, t0, t1;
        QueryPerformanceFrequency(&f);
        QueryPerformanceCounter(&t0);
#else
        timeval t0, t1, delta;
        gettimeofday(&t0, NULL);
#endif

        engine->service->tick();

#ifdef WIN32
        QueryPerformanceCounter(&t1);
        if (TICKS * (t1.QuadPart - t0.QuadPart) > f.QuadPart) {
#else
        gettimeofday(&t1, NULL);
        timersub(&t1, &t0, &delta);
        if (delta.tv_sec > 0 || delta.tv_usec > 1000000 / TICKS) {
#endif
            LOG4CPLUS_WARN(core::log,
                "Game engine is behind! Loop took "
#ifdef WIN32
                << ((t1.QuadPart - t0.QuadPart) / f.QuadPart) << " seconds, " << (((t1.QuadPart - t0.QuadPart) * 1000000 / f.QuadPart) % 1000000) << " microseconds.");
#else
                << delta.tv_sec << " seconds, " << delta.tv_usec << " microseconds.");
#endif
        }
        else {
#ifdef WIN32
            Sleep(1000 / TICKS - (t1.QuadPart - t0.QuadPart) * 1000 / f.QuadPart);
#else
            timespec ts, tr;
            ts.tv_sec = 0;
            ts.tv_nsec = 999999999 / TICKS - (delta.tv_usec * 1000);
            while (nanosleep(&ts, &tr) != 0) {
                LOG4CPLUS_WARN(core::log,
                    "Unexpected interruption occurred in GameEngine::gameLoop");
                ts.tv_sec = tr.tv_sec;
                ts.tv_nsec = tr.tv_nsec;
            }
#endif
        }
    }

    return 0;
}

bool GameEngine::isRunning()
{
    return running;
}
