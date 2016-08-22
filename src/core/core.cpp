#include "core.h"

log4cplus::Logger core::log()
{
	return log4cplus::Logger::getInstance("renaissance");
}

