#pragma once
#include "CommonConcepts.h"
#include <cmath>

namespace sampling
{
	enum class SamplerMode
	{
		Nearest,
		Bilinear
	};

	template<typename T, con::InvocableWith<int, int> SampleTexelT>
	[[nodiscard]]
	T bilinear(float u, float v, ivec2 dimensions, SampleTexelT sample)
	{
		const float texelX = u * float(dimensions.x());
		const float texelY = v * float(dimensions.y());
		const int texelLeft = static_cast<int>(texelX);
		const int texelDown = static_cast<int>(texelY);
		const int texelRight = texelLeft + 1;
		const int texelUp = texelDown + 1;

		const T topLeft = sample(texelLeft, texelUp);
		const T topRight = sample(texelRight, texelUp);
		const T bottomLeft = sample(texelLeft, texelDown);
		const T bottomRight = sample(texelRight, texelDown);

		float _;
		const float xDecimal = modf(texelX, &_);
		const float yDecimal = modf(texelY, &_);

		using std::lerp;
		return lerp(lerp(topLeft, topRight, xDecimal), lerp(bottomLeft, bottomRight, xDecimal), yDecimal);
	}
}