#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED
#include <limits>
#include "Utilities.h"
#include "vec.h"

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)


class AABB
{
public:
	AABB(const vec3 &givenMin, const vec3 &givenMax) : min(givenMin), max(givenMax) {}
	AABB(){}

	static AABB unite(const AABB &firstBox, const AABB &secondBox)
	{
		vec3 min = {}, max = {};
		for (size_t i = 0; i < 3; i++)
		{
			min[i] = _min(firstBox.min[i], secondBox.min[i]);
			max[i] = _max(firstBox.max[i], secondBox.max[i]);
		}
		return AABB(min, max);
	}

	AABB &boundInto(const AABB &other)
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

	vec3 min, max;
};


#endif // !AABB_H_DEFINED
