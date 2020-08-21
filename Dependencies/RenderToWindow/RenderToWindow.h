#ifndef RENDER_TO_WINDOW_H_DEFINED
#define RENDER_TO_WINDOW_H_DEFINED
#include <cstdint>
#pragma system_header

union color
{
	//4201 relates to a nonstandard extension, namely "nameless struct/union"
	//in this case, I'm just writing this for msvc so I don't mind
#pragma warning (push)
#pragma warning (disable:4201)
	struct
	{
		uint8_t b, g, r, a;
	};
	uint8_t data[4];
#pragma warning(pop)
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