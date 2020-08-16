#pragma once
#include "AABB.h"
#include <utility>

struct Triangle
{
	vec3 v1, v2, v3;
	vec3 c1, c2, c3;
	vec3 us, vs;
	vec3 n1, n2, n3;

	[[nodiscard]]
	AABB calculateAABB() const
	{
		vec3 min = v1, max = v1;

		for (size_t i = 0; i < 3; i++)
		{
			min[i] = _min(v1[i], v2[i]);
			min[i] = _min(min[i], v3[i]);

			max[i] = _max(v1[i], v2[i]);
			max[i] = _max(max[i], v3[i]);
		}

		return AABB(min, max);
	}

	[[nodiscard]]
	vec3 calculate2DBarycentricCoords(const vec3 &point) const
	{
		vec3 s[2];
		for (int i = 2; i--; ) {
			s[i][0] = v3[i] - v1[i];
			s[i][1] = v2[i] - v1[i];
			s[i][2] = v1[i] - point[i];
		}
		vec3 u = vec3::cross(s[0], s[1]);
		if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
			return vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
		return vec3(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
	}

	[[nodiscard]]
	vec3 calculateFaceNormal() const
	{
		return vec3::cross(v2 - v1, v3 - v1).normalized();
	}
};