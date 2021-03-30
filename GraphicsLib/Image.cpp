#include "Image.h"

#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vec3 Image::atUV(float u, float v, sampling::SamplerMode mode) const
{
	auto nearest = [this](float u, float v)
	{
		const int texelX = int(u * dimensions.x());
		const int texelY = int(v * dimensions.y());
		return atTexel(texelX, texelY);
	};

	const color rgb = [=]()
		{
			switch (mode)
			{
			case sampling::SamplerMode::Nearest:
			{
				return nearest(u, v);
			}break;
			case sampling::SamplerMode::Bilinear:
			{
				auto sample = [this](int x, int y) { return atTexel(x, y); };
				//todo: revise sampling code so it has nice template argument deduction
				return sampling::bilinear<color, decltype(sample)>(u, v, dimensions, sample);
			}break;
			default:
				assert(false);
				return color{};
			}
		}(); //immediately invoked

	return vec3(
		float(rgb.r) / 255.0f,
		float(rgb.g) / 255.0f,
		float(rgb.b) / 255.0f
	);
}

color Image::atTexel(int texelX, int texelY) const
{
	texelX %= dimensions.x();
	texelY %= dimensions.y();
	texelY = dimensions.y() - texelY;
	unsigned char* colorPtr = &data[3 * texelX + 3 * dimensions.x() * texelY];
	return { .b = colorPtr[2], .g = colorPtr[1], .r = colorPtr[0], .a = colorPtr[3] };
}

Image::Image(const char *path)
{
	int nn{0};
	data = stbi_load(path, &dimensions.x(), &dimensions.y(), &nn, STBI_rgb);
}

Image::~Image()
{
	stbi_image_free(data);
}

