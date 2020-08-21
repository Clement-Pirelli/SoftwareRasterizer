#pragma once
#include "AABB.h"
#include <utility>
#include "Utilities.h"

struct BarycentricCoordinates
{
	BarycentricCoordinates(float a, float b, float c)
	{
		coordinates[0] = a;
		coordinates[1] = b;
		coordinates[2] = c;
	}

	bool areDegenerate() const 
	{
		for(size_t i = 0; i < 3; i++)
		{
			if (coordinates[i] < .0f) return true;
		}
		return false;
	}

	template<class T>
	T weigh(const T &firstValue, const T &secondValue, const T &thirdValue) const
	{
		return firstValue * coordinates[0] + secondValue * coordinates[1] + thirdValue * coordinates[2];
	}

	float &operator[](size_t i) noexcept
	{
		return coordinates[i];
	}

	float operator[](size_t i) const noexcept
	{
		return coordinates[i];
	}

private:
	float coordinates[3] = {};
};

struct Triangle
{
	struct Vertex
	{
		vec3 position;
		vec3 color;
		float u, v;
		vec3 normal;
	};
	
	Vertex vertices[3];

	[[nodiscard]]
	AABB calculateAABB() const noexcept
	{
		vec3 min = vertices[0].position, max = vertices[0].position;

		for (size_t i = 0; i < 3; i++)
		{
			min[i] = _min(vertices[0].position[i], vertices[1].position[i]);
			min[i] = _min(min[i], vertices[2].position[i]);

			max[i] = _max(vertices[0].position[i], vertices[1].position[i]);
			max[i] = _max(max[i], vertices[2].position[i]);
		}

		return AABB(min, max);
	}

	//taken from : https://github.com/ssloy/tinyrenderer/
	[[nodiscard]]
	BarycentricCoordinates calculate2DBarycentricCoords(const vec3 &point) const noexcept
	{
		vec3 s[2];
		for (int i = 2; i--; ) {
			s[i][0] = vertices[2].position[i] - vertices[0].position[i];
			s[i][1] = vertices[1].position[i] - vertices[0].position[i];
			s[i][2] = vertices[0].position[i] - point[i];
		}
		vec3 u = vec3::cross(s[0], s[1]);
		if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
			return BarycentricCoordinates(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
		return BarycentricCoordinates(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
	}

	[[nodiscard]]
	vec3 calculateFaceNormal() const noexcept
	{
		return vec3::cross(vertices[1].position - vertices[0].position, vertices[2].position - vertices[0].position).normalized();
	}
};