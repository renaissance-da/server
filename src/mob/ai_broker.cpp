/*
 * ai_broker.cpp
 *
 *  Created on: Apr 19, 2014
 *      Author: per
 */

#include "ai_broker.h"

#include <map>

typedef std::pair<std::function<MobAI * (int, Mob *)>, std::string> aiGen;
std::map<int, aiGen> gens;

/**
 * \brief Register an AI with the system.
 *
 * Registers an AI with the system. Users can then allocate instances of that
 * AI by specifying the code to be used.
 *
 * \param[in] code The code to register the AI generator with.
 * \param[in] aiType A string used to describe which AI is being registered.
 * \param[in] gen A function which will be used to generate the requested AI.
 * This function may throw AIBroker exceptions, but should not throw anything
 * else.
 * \throws AlreadyRegisteredException if the code requested has already been
 * registered.
 */
void mob::registerAI(int code, std::string aiType, std::function<MobAI * (int, Mob *)> gen)
{
	auto prev = gens.find(code);
	if (prev == gens.end()) {
		//Register
		gens[code] = aiGen{ gen, aiType };
	}
	else {
		//Already in use
		throw AlreadyRegisteredException(code, prev->second.second, aiType);
	}
}

/**
 * \brief Generates the requested AI.
 *
 * Generate an AI, identified by the specified type.
 *
 * \param[in] code The code identifying the type of AI to be generated.
 * \param[in] param A parameter to be passed to the generator.
 * \param[in] mob The mob to attach the new AI to.
 *
 * \return A dynamically allocated mob AI.
 */
MobAI *mob::getAI(int code, int param, Mob *mob)
{
	auto aiInfo = gens.find(code);

	if (aiInfo != gens.end()) {
		return aiInfo->second.first(param, mob);
	}

	//Not found
	throw NotRegisteredException(code);
}
