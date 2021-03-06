#pragma once
#include "AABB.h"
#include <utility>
#include "Utilities.h"
#include <array>

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)

struct Triangle
{
	struct BarycentricCoordinates
	{
		friend struct Triangle;
		bool areDegenerate() const
		{
			for (size_t i = 0; i < 3; i++)
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
		BarycentricCoordinates() = delete;

		BarycentricCoordinates(float a, float b, float c)
		{
			coordinates[0] = a;
			coordinates[1] = b;
			coordinates[2] = c;
		}

		vec3 coordinates = {};
	};


	struct Vertex
	{
		vec4 position;
		vec3 color;
		float u, v;
		vec3 normal;

		static Vertex barycentricInterpolation(BarycentricCoordinates coords, const Vertex& a, const Vertex& b, const Vertex& c)
		{
			const vec4 pos = coords.weigh(
				a.position,
				b.position,
				c.position
			);

			const vec3 vertexCol = coords.weigh(
				a.color,
				b.color,
				c.color
			);

			const vec3 normal = coords.weigh(
				a.normal,
				b.normal,
				c.normal
			).normalized();


			const float u = coords.weigh(
				a.u,
				b.u,
				c.u
			);

			const float v = coords.weigh(
				a.v,
				b.v,
				c.v
			);

			return
			{
				.position = pos,
				.color = vertexCol,
				.u = u,
				.v = v,
				.normal = normal
			};
		}
	};
	
	Vertex vertices[3];

	[[nodiscard]]
	AABB2 calculateAABB2() const noexcept
	{
		vec2 min = vertices[0].position.xy(), max = vertices[0].position.xy();

		for (size_t i = 0; i < vec2::size(); i++)
		{
			min[i] = _min(vertices[0].position[i], vertices[1].position[i]);
			min[i] = _min(min[i], vertices[2].position[i]);

			max[i] = _max(vertices[0].position[i], vertices[1].position[i]);
			max[i] = _max(max[i], vertices[2].position[i]);
		}

		return AABB2(min, max);
	}

	//taken from : https://github.com/ssloy/tinyrenderer/
	[[nodiscard]]
	BarycentricCoordinates calculate2DBarycentricCoords(const vec2 &point, std::array<float, 3> vertexWs) const noexcept
	{
		vec3 s[2];
		for (int i = 2; i--; ) {
			s[i][0] = vertices[2].position[i] - vertices[0].position[i];
			s[i][1] = vertices[1].position[i] - vertices[0].position[i];
			s[i][2] = vertices[0].position[i] - point[i];
		}
		vec3 u = vec3::cross(s[0], s[1]);
		if (std::abs(u.z()) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		{
			auto coordinates = BarycentricCoordinates(1.f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
			
			for(int i = 0; i < 3; i++)
			{
				if(vertexWs[i] >= .0001f) coordinates.coordinates[i] /= vertexWs[i];
			}

			return coordinates;
		}
		return BarycentricCoordinates(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizer
	}

	[[nodiscard]]
	vec3 calculateFaceNormal() const noexcept
	{
		return vec3::cross((vertices[1].position - vertices[0].position).xyz(), (vertices[2].position - vertices[0].position).xyz()).normalized();
	}
};


#undef _min
#undef _max