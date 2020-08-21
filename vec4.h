#pragma once
#include "vec3.h"
union vec4
{
	vec4() = default;
	vec4(float e0, float e1, float e2, float e3) : e{ e0,e1,e2,e3 } {}
	inline float operator[](int i) const { return e[i]; }
	inline float &operator[](int i) { return e[i]; }
	inline vec4 operator/(float value) { return vec4(x/value, y/value, z/value, w/value); }
	vec3 xyz() { return vec3(x,y,z); }

	struct
	{
		float x, y, z, w;
	};

	struct 
	{
		float r, g, b, a;
	};

	static float dot(const vec4 &a, const vec4 &b)
	{
		float result = .0f;
		for(size_t i = 0; i < 4; i++)
		{
			result += a[i] * b[i];
		}
		return result;
	}

	float e[4] = {};


	static vec4 fromPoint(const vec3 &point) { return { point.x, point.y, point.z, 1.0f }; }
	static vec4 fromDirection(const vec3 &direction) { return { direction.x, direction.y, direction.z, .0f }; };
};

