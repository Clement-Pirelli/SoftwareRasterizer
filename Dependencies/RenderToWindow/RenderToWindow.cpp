#include "RenderToWindow.h"
#include "Logger/Logger.h"
#pragma system_header

#pragma region WINDOW DISPLAY

//base code for window display provided Tommi Lipponen
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <stdio.h>

struct RenderTarget {
	HDC device;
	size_t width;
	size_t height;
	unsigned int *data;
	BITMAPINFO info;

	RenderTarget(HDC givenDevice, size_t width, size_t height)
		: device(givenDevice), width(width), height(height), data(nullptr)
	{
		data = new unsigned int[width * height];
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biWidth = static_cast<LONG>(width);
		info.bmiHeader.biHeight = static_cast<LONG>(height);
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 32;
		info.bmiHeader.biCompression = BI_RGB;
	}
	~RenderTarget() { delete[] data; }
	inline int  size() const {
		return width * height;
	}
	void clear(unsigned color) {
		const int count = size();
		for (int i = 0; i < count; i++) {
			data[i] = color;
		}
	}
	inline void pixel(int x, int y, unsigned color) {
		data[y * width + x] = color;
	}
	void present() {
		StretchDIBits(device,
			0, 0, width, height,
			0, 0, width, height,
			data, &info,
			DIB_RGB_COLORS, SRCCOPY);
	}
};

static unsigned
makeColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	unsigned result = 0;
	if (alpha > 0)
	{
		result |= ((unsigned)red << 16);
		result |= ((unsigned)green << 8);
		result |= ((unsigned)blue << 0);
		result |= ((unsigned)alpha << 24);
	}
	return result;
}

static LRESULT CALLBACK
Win32DefaultProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_CLOSE: { PostQuitMessage(0); } break;
	default: { return DefWindowProcA(window, message, wparam, lparam); } break;
	}
	return 0;
}

#pragma endregion


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;
	default:
		return DefWindowProcA(hWnd, message, wParam, lParam);
	}
}

RenderToWindow::RenderToWindow(size_t width, size_t height, const char *title) : width(width), height(height), rt(nullptr)
{
	const char *const myclass = "minimalWindowClass";

	WNDCLASSEXA wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = GetModuleHandleA(0);
	wc.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(32512)); //arrow
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = myclass;

	if (RegisterClassExA(&wc))
	{
		DWORD window_style = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
		RECT rc = { 0, 0, width, height };
		if (!AdjustWindowRect(&rc, window_style, FALSE))
		{
			Logger::logError("Couldn't show the image : window rect adjustment failed!");
			return;
		}
		HWND windowHandleA = CreateWindowExA(0, wc.lpszClassName,
			title, window_style, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, GetModuleHandleA(NULL), NULL);
		if (!windowHandleA) { Logger::logError("Couldn't show the image : window handle creation was unsuccessful!"); return; }
		ShowWindow(windowHandleA, SW_SHOW);

		if (windowHandleA)
		{
			HDC device = GetDC(windowHandleA);
			rt = new RenderTarget(device, width, height);

			rt->clear(makeColor(0x00, 0x00, 0x00, 0xff));
		}
		else
		{
			Logger::logError("Couldn't show the image : Couldn't create the window!");
		}
	}
	else
	{
		Logger::logError("Couldn't show the image : Could not register window class!");
	}
}

RenderToWindow::~RenderToWindow()
{
	delete rt;
}

void RenderToWindow::handleMessagesBlocking()
{
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0))
	{
		if (msg.message == WM_QUIT) { return; }
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		rt->present();
	}
}

void RenderToWindow::updateImage(color *image)
{
	if (rt == nullptr) return;


	for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width; x++)
		{
			color &currentColor = image[y * width + x];
			rt->pixel(x, y, makeColor(currentColor.r, currentColor.g, currentColor.b, currentColor.a));
		}

	rt->present();
}
