/*
 * GameEngine.h
 *
 * Runs the game loop for dataservice
 *  Created on: 2012-12-13
 *      Author: per
 */

#ifndef GAMEENGINE_H_
#define GAMEENGINE_H_

#include "DataService.h"
#include "random.h"
#include <thread>
#include <stdlib.h>

class GameEngine
{
public:
    GameEngine(DataService *s, drand48_data *rng);
    ~GameEngine();bool isRunning();

private:
    DataService *service;
    std::thread thread;
	bool running;
    drand48_data *rng;

    static void gameLoop(GameEngine *engine);

};

#endif /* GAMEENGINE_H_ */
