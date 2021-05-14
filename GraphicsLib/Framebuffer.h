#pragma once
#include <cmath>
#include "AABB.h"
#include "Sampling.h"
#include <assert.h>

namespace gl
{
	template<class T>
	class FrameBuffer
	{
	public:

		struct CreateInfo
		{
			size_t width = 0;
			size_t height = 0; 
			T clearValue = {};
		};

		FrameBuffer(const CreateInfo &info) :
			width(info.width),
			height(info.height),
			data(new T[info.width * info.height]),
			clearValue(info.clearValue)
		{
			clear();
		}

		~FrameBuffer()
		{
			delete[] data;
		}

		[[nodiscard]]
		const T& atTexel(size_t x, size_t y) const
		{
			return data[x + width * y];
		}

		[[nodiscard]]
		T &atTexel(size_t x, size_t y)
		{
			return data[x + width * y];
		}

		[[nodiscard]]
		T &atIndex(size_t index)
		{
			return data[index];
		}

		[[nodiscard]]
		T atUV(float u, float v, sampling::SamplerMode mode = sampling::SamplerMode::Nearest) const
		{
			float _;
			u = modf(u, &_);
			v = modf(v, &_);
			if (u < .0f) u += 1.0f;
			if (v < .0f) v += 1.0f;

			switch (mode)
			{
			case sampling::SamplerMode::Nearest:
			{
				return atTexel(size_t(u * width), size_t(v * height));
			} break;
			case sampling::SamplerMode::Bilinear:
			{
				const ivec2 dimensions = ivec2(static_cast<int>(width), static_cast<int>(height));
				auto sampleTexel = [this](int x, int y)
				{
					assert(x >= 0 && y >= 0);
					return atTexel(size_t(x), size_t(y));
				};
				return sampling::bilinear<T, decltype(sampleTexel)>(u, v, dimensions, sampleTexel);

			} break;
			default:
				assert(false);
				return T{};
			}
		}

		void clear()
		{
			for (size_t i = 0; i < width * height; i++)
			{
				data[i] = clearValue;
			}
		}

		size_t width = {}, height = {};
		T *data = nullptr;

		[[nodiscard]]
		AABB2 bounds() const
		{
			vec2 min = vec2(0.0f, 0.0f);
			vec2 max = vec2(static_cast<float>(width), static_cast<float>(height));
			return AABB2(min, max);
		}

		const T clearValue;
	};
}
