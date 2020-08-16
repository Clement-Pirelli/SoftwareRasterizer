#pragma once
#include <functional>

class CoreLoop
{
public:
	void run(std::function<void()> update) const;
};