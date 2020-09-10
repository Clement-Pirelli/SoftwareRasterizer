#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED
#include <limits>
#include "Utilities.h"
#include "vec.h"

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)


class AABB2
{
public:
	AABB2(const vec2 &givenMin, const vec2 &givenMax) : min(givenMin), max(givenMax) {}
	AABB2(){}

	static AABB2 unite(const AABB2 &firstBox, const AABB2 &secondBox)
	{
		vec2 min = {}, max = {};
		for (size_t i = 0; i < 2; i++)
		{
			min[i] = _min(firstBox.min[i], secondBox.min[i]);
			max[i] = _max(firstBox.max[i], secondBox.max[i]);
		}
		return AABB2(min, max);
	}

	AABB2 &boundInto(const AABB2 &other)
	{
		min = min.clampedBy(other.min, other.max);
		max = max.clampedBy(other.min, other.max);
		return *this;
	}

	bool isPoint() const
	{
		return isApproximatively(min.x(), max.x(), std::numeric_limits<float>::epsilon()) &&
			isApproximatively(min.y(), max.y(), std::numeric_limits<float>::epsilon());
	}

	vec2 min, max;
};


#endif // !AABB_H_DEFINED
