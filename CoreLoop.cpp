#include "CoreLoop.h"
#include <Windows.h>


input::InputState onKeyDown(input::InputState lastInput)
{
	switch(lastInput)
	{
	case input::InputState::Repeated:
	case input::InputState::Pressed:
		return input::InputState::Repeated;
	default:
		return input::InputState::Pressed;
	}
}

input::InputState onKeyUp(input::InputState lastInput)
{
	switch (lastInput)
	{
	case input::InputState::Repeated:
	case input::InputState::Pressed:
		return input::InputState::Released;
	default:
		return input::InputState::Idle;
	}
}

std::optional<Input> CoreLoop::emptyQueue()
{
	static Input lastInput = {};
	Input input = lastInput;

	MSG msg;
	while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
	{
		const input::VirtualKeys key = (input::VirtualKeys)msg.wParam;

		switch(msg.message)
		{
		case WM_QUIT:
		case WM_CLOSE:
			return std::nullopt;
		case WM_KEYDOWN:
		{
			input[key] = onKeyDown(lastInput[key]);
		} break;
		case WM_KEYUP: 
		{
			input[key] = onKeyUp(lastInput[key]);
		} break;
		}

		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	return input;
}
