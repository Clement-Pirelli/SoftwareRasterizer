#pragma once
#include "vec3.h"

struct Image
{
	unsigned char *data = nullptr;
	int x = 0, y = 0;

	vec3 atUV(float u, float v) const
	{
		const int texelX = int(u * x);
		const int texelY = int(v * y);
		unsigned char *rgb = atTexel(texelX, texelY);

		vec3 color = vec3(
			float(rgb[0]) / 255.0f,
			float(rgb[1]) / 255.0f,
			float(rgb[2]) / 255.0f
		);

		return color;
	}

	unsigned char *atTexel(int texelX, int texelY) const
	{
		texelX %= x;
		texelY %= y;
		texelY = y - texelY;
		return &data[3 * texelX + 3 * x * texelY];
	}
};