#pragma once
#include "vec.h"


class Image
{
public:
	vec3 atUV(float u, float v) const;

	unsigned char *atTexel(int texelX, int texelY) const;

	explicit Image(const char *path);
	~Image();

private:
	unsigned char *data = nullptr;
	int x = 0, y = 0;

	Image() = delete;
};