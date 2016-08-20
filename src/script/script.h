/*
 * script.h
 *
 *  Created on: Apr 27, 2014
 *      Author: per
 */

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"

#define REGISTER(TOKEN) lua_register(L, "NPC_" #TOKEN, TOKEN)

namespace script
{
    log4cplus::Logger log();
}


#endif /* SCRIPT_H_ */
