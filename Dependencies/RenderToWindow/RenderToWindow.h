#ifndef RENDER_TO_WINDOW_H_DEFINED
#define RENDER_TO_WINDOW_H_DEFINED
#include "color.h"
#pragma system_header

struct RenderTarget;

class RenderToWindow
{
public:
	RenderToWindow(size_t width, size_t height, const char *title);
	~RenderToWindow();

	void handleMessagesBlocking();
	void updateImage(color *image);
private:
	size_t width, height;
	RenderTarget *rt;
	RenderToWindow() = delete;
};

#endif