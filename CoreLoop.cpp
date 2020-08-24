#include "CoreLoop.h"
#include <Windows.h>

void CoreLoop::run(std::function<void(const Time &)> update){
	while(true)
	{
		MSG msg;
		while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) { return; }
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		update(Time::now());
	}
}
