#pragma once
#include <cstdint>

struct color
{
	uint8_t b, g, r, a;
};

inline static color lerp(color first, color second, float t)
{
	auto lerpElement = [](uint8_t a, uint8_t b, float t)
	{
		return uint8_t(a * (1.0f - t) + b * t);
	};

	return color
	{
		.b = lerpElement(first.b, second.b, t),
		.g = lerpElement(first.g, second.g, t),
		.r = lerpElement(first.r, second.r, t),
		.a = lerpElement(first.a, second.a, t)
	};
}