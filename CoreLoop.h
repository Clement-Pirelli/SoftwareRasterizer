#pragma once
#include <functional>
#include "Time.h"
class CoreLoop
{
public:
	void run(std::function<void(const Time &)> update) const;
};