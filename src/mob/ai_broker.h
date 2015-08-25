/*
 * ai_broker.h
 *
 *  Created on: Apr 19, 2014
 *      Author: per
 */

#ifndef AI_BROKER_H_
#define AI_BROKER_H_

#include <string>
#include <functional>
#include "Mob.h"
#include "MobAI.h"

namespace mob {

class AIBrokerException {
public:
	std::string message;
};

class AlreadyRegisteredException : public AIBrokerException {
public:
	int codeRequested;
	std::string oldType, newType;

	AlreadyRegisteredException(int codeRequested,
			std::string oldType,
			std::string newType):
	codeRequested(codeRequested),
	oldType(oldType),
	newType(newType)
	{
		message = "Attempted to register two AI's with the same code!";
	}
};

class InvalidParameterException : public AIBrokerException {
public:
	int paramRequested;

	InvalidParameterException(int param, std::string message):
	paramRequested(param)
	{
		this->message = message;
	}
};

class NotRegisteredException : public AIBrokerException {
public:
	int code;

	NotRegisteredException(int code):
	code(code)
	{
		this->message = message;
	}
};


void registerAI(int code, std::string aiType, std::function<MobAI * (int, Mob *)> gen);
MobAI *getAI(int code, int param, Mob *mob);

} //namespace Mob

#endif /* AI_BROKER_H_ */
