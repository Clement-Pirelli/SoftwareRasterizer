#pragma once
#include "vec.h"
#include <cstdint>
#include "color.h"
#include "Sampling.h"

class Image
{
public:

	vec3 atUV(float u, float v, sampling::SamplerMode mode = sampling::SamplerMode::Bilinear) const;

	color atTexel(int texelX, int texelY) const;

	explicit Image(const char *path);
	~Image();

private:
	unsigned char *data = nullptr;
	ivec2 dimensions{};

	Image() = delete;
};