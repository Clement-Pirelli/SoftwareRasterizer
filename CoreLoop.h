#pragma once
#include <functional>
#include "Time.h"
class CoreLoop
{
	CoreLoop() = delete;
public:
	static void run(std::function<void(const Time &)> update);
};