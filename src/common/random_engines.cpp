#include "random_engines.h"
#include <chrono>

thread_local std::mt19937 *local_generator = nullptr;

std::mt19937 &generator()
{
    typedef std::chrono::duration<uint_fast32_t> duration;
    
    if (local_generator == nullptr) {
	auto seconds_since_epoch =
	    std::chrono::duration_cast<duration>(
		std::chrono::system_clock::now().time_since_epoch());
	local_generator = new std::mt19937(seconds_since_epoch.count());
    }

    return *local_generator;
}
