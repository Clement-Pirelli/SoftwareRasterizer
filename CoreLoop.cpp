#include "CoreLoop.h"
#include <Windows.h>

void CoreLoop::run(std::function<void()> update) const
{
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (msg.message == WM_QUIT) { return; }
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		update();
	}
}
