#pragma once

template<class T>
class FrameBuffer
{
public:
	FrameBuffer(size_t givenWidth, size_t givenHeight, const T &clearValue) :
		width(givenWidth),
		height(givenHeight),
		data(new T[width * height]),
		bounds(calculateBounds())
	{
		clear(clearValue);
	}

	~FrameBuffer()
	{
		delete[] data;
	}

	[[nodiscard]]
	T &at(size_t x, size_t y) const
	{
		return data[x + width * y];
	}

	[[nodiscard]]
	T &at(size_t index) const
	{
		return data[index];
	}

	void clear(const T &clearValue)
	{
		for (size_t i = 0; i < width * height; i++)
		{
			data[i] = clearValue;
		}
	}

	size_t width = {}, height = {};
	T *data = nullptr;
	const AABB bounds;

private:
	[[nodiscard]]
	AABB calculateBounds() const
	{
		vec3 min = vec3(0.0f, 0.0f, std::numeric_limits<float>::min());
		vec3 max = vec3(static_cast<float>(width), static_cast<float>(height), std::numeric_limits<float>::max());
		return AABB(min, max);
	}
};