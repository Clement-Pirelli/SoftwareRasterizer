#ifndef RENDER_TO_WINDOW_H_DEFINED
#define RENDER_TO_WINDOW_H_DEFINED
#include <cstdint>

union color
{
	struct
	{
		uint8_t b, g, r, a;
	};
	uint8_t data[4];
};

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