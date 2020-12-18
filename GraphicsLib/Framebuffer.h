#pragma once
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
			bounds(calculateBounds()),
			clearValue(info.clearValue)
		{
			clear();
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

		void clear()
		{
			for (size_t i = 0; i < width * height; i++)
			{
				data[i] = clearValue;
			}
		}

		size_t width = {}, height = {};
		T *data = nullptr;
		const AABB2 bounds;
		const T clearValue;

	private:
		[[nodiscard]]
		AABB2 calculateBounds() const
		{
			vec2 min = vec2(0.0f, 0.0f);
			vec2 max = vec2(static_cast<float>(width), static_cast<float>(height));
			return AABB2(min, max);
		}
	};
}
