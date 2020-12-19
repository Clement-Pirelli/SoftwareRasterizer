#ifndef RENDER_TO_WINDOW_H_DEFINED
#define RENDER_TO_WINDOW_H_DEFINED

#include "vec.h"
#pragma system_header

struct RenderTarget;

class RenderToWindow
{
public:
	RenderToWindow(size_t width, size_t height, const char *title);
	~RenderToWindow();

	void handleMessagesBlocking();
	void updateImage(vec4 *image);
	void updateImage(float *image);
private:
	size_t width, height;
	RenderTarget *rt;
	RenderToWindow() = delete;
};

#endif