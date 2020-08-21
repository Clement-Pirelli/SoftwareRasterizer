#pragma once
#include <cstdint>

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