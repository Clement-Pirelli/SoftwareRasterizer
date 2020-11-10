#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vec3 Image::atUV(float u, float v) const
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

unsigned char *Image::atTexel(int texelX, int texelY) const
{
	texelX %= x;
	texelY %= y;
	texelY = y - texelY;
	return &data[3 * texelX + 3 * x * texelY];
}

Image::Image(const char *path)
{
	int nn{0};
	data = stbi_load(path, &x, &y, &nn, STBI_rgb);
}

Image::~Image()
{
	stbi_image_free(data);
}

