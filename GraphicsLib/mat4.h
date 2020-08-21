#pragma once
#include <array>
#include "vec4.h"
#include <math.h>

struct mat4
{
private:
	struct MatrixAsVectors
	{
		MatrixAsVectors(const mat4 &m)
		{
			vecs[0] = vec4(m[0], m[1], m[2], m[3]);
			vecs[1] = vec4(m[4], m[5], m[6], m[7]);
			vecs[2] = vec4(m[8], m[9], m[10], m[11]);
			vecs[3] = vec4(m[12], m[13], m[14], m[15]);
		}

		vec4 operator[](size_t i)
		{
			return vecs[i];
		}

		std::array<vec4, 4> vecs;
	};
public:



	mat4() { *this = identity();  }
	mat4(std::array<float, 16> arr)
	{
		elements = arr;
	}

	std::array<float, 16> elements = {};

	inline float operator[](size_t i) const
	{
		return elements[i];
	}

	inline float &operator[](size_t i)
	{
		return elements[i];
	}

	inline vec4 operator *(const vec4 &v) const
	{
		MatrixAsVectors asVectors(*this);
		return vec4(
			vec4::dot(asVectors.vecs[0], v),
			vec4::dot(asVectors.vecs[1], v),
			vec4::dot(asVectors.vecs[2], v),
			vec4::dot(asVectors.vecs[3], v)
		);
	}

	inline mat4 operator *(const float value) const
	{
		mat4 result = *this;
		for (auto &element : result.elements)
		{
			element *= value;
		}
		return result;
	}

	inline mat4 operator /(const float value) const
	{
		mat4 result = *this;
		for(auto &element : result.elements)
		{
			element /= value;
		}
		return result;
	}

	inline mat4 operator *(const mat4 &other) const
	{
		mat4 result = empty();

		MatrixAsVectors thisAsVectors(*this);
		MatrixAsVectors otherAsVectors(transposed(other));

		for(size_t y = 0; y < 4U; y++)
		for(size_t x = 0; x < 4U; x++)
		{
			result[x + 4U * y] = vec4::dot(thisAsVectors[x], otherAsVectors[y]);
		}

		return result;
	}

	inline static mat4 transposed(const mat4 mat)
	{
		return mat4({
			mat[0], mat[4], mat[8], mat[12],
			mat[1], mat[5], mat[9], mat[13],
			mat[2], mat[6], mat[10], mat[14],
			mat[3], mat[7], mat[11], mat[15],
			});
	}

	inline static mat4 rotateX(float byRadians)
	{
		return mat4({
			1.0f, .0f,			 .0f,				 .0f,
			 .0f, cosf(byRadians), -sinf(byRadians), .0f,
			 .0f, sinf(byRadians), cosf(byRadians),  .0f,
			 .0f, .0f,			 .0f,				1.0f
			});
	}

	inline static mat4 rotateY(float byRadians)
	{
		return mat4({
			cosf(byRadians), .0f, sinf(byRadians),	 .0f,
			.0f,			1.0f, .0f,				 .0f,
			-sinf(byRadians),.0f, cosf(byRadians),	 .0f,
			.0f,			 .0f, .0f,				1.0f
			});
	}

	inline static mat4 rotateZ(float byRadians)
	{
		return mat4({
			cosf(byRadians), -sinf(byRadians),	 .0f, .0f,
			sinf(byRadians), cosf(byRadians),	 .0f, .0f,
			.0f,			.0f,				1.0f, .0f,
			.0f,			.0f,				 .0f, 1.0f
			});
	}

	inline static mat4 translate(const vec3 &by)
	{
		return mat4({
			1.0f, .0f, .0f, by.x,
			.0f, 1.0f, .0f, by.y,
			.0f, .0f, 1.0f,	by.z,
			.0f, .0f, .0f , 1.0f
			});
	}
	
	inline static mat4 scale(const vec3 &by)
	{
		return mat4({
			by.x, .0f, .0f, .0f,
			.0f, by.y, .0f, .0f,
			.0f, .0f, by.z, .0f,
			.0f, .0f, .0f, 1.0f
			});
	}

	struct Viewport
	{
		float x = 0, y = 0;
		size_t width = 0, height = 0;
	};
	inline static mat4 viewport(const Viewport &viewport)
	{
		const float halfWidth = static_cast<float>(viewport.width) / 2.0f;
		const float halfHeight = static_cast<float>(viewport.height) / 2.0f;
		return mat4({
			halfWidth, .0f,.0f,viewport.x + halfWidth,
			.0f, halfHeight, .0f, viewport.y + halfHeight,
			.0f,.0f,1.0f,.0f,
			.0f,.0f,.0f,1.0f
			});
	}

	inline static mat4 identity()
	{
		return mat4({
			1.0f, .0f, .0f, .0f,
			.0f, 1.0f, .0f, .0f,
			.0f, .0f, 1.0f, .0f,
			.0f, .0f, .0f, 1.0f
		});
	}

	//from : https://github.com/glslify/glsl-inverse/blob/master/index.glsl
	inline static mat4 inversed(const mat4 &mat)
	{
		const float
			a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3],
			a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7],
			a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11],
			a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15],

			b00 = a00 * a11 - a01 * a10,
			b01 = a00 * a12 - a02 * a10,
			b02 = a00 * a13 - a03 * a10,
			b03 = a01 * a12 - a02 * a11,
			b04 = a01 * a13 - a03 * a11,
			b05 = a02 * a13 - a03 * a12,
			b06 = a20 * a31 - a21 * a30,
			b07 = a20 * a32 - a22 * a30,
			b08 = a20 * a33 - a23 * a30,
			b09 = a21 * a32 - a22 * a31,
			b10 = a21 * a33 - a23 * a31,
			b11 = a22 * a33 - a23 * a32,

			det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

		return mat4(
			{
			a11 *b11 - a12 * b10 + a13 * b09, a02 *b10 - a01 * b11 - a03 * b09, a31 *b05 - a32 * b04 + a33 * b03, a22 *b04 - a21 * b05 - a23 * b03, 
			a12 *b08 - a10 * b11 - a13 * b07, a00 *b11 - a02 * b08 + a03 * b07, a32 *b02 - a30 * b05 - a33 * b01, a20 *b05 - a22 * b02 + a23 * b01, 
			a10 *b10 - a11 * b08 + a13 * b06, a01 *b08 - a00 * b10 - a03 * b06, a30 *b04 - a31 * b02 + a33 * b00, a21 *b02 - a20 * b04 - a23 * b00, 
			a11 *b07 - a10 * b09 - a12 * b06, a00 *b09 - a01 * b07 + a02 * b06, a31 *b01 - a30 * b03 - a32 * b00, a20 *b03 - a21 * b01 + a22 * b00
			}) / det;
	}



	struct PerspectiveProjection
	{
		float fovX = {}, aspectRatio = {}, zfar = {}, znear = {};
	};

	inline static mat4 perspective(const PerspectiveProjection &projection)
	{
		const float fovY = projection.fovX * projection.aspectRatio;
		const float halfFovY = fovY * .5f;
		const float halfFovX = projection.fovX * .5f;

		const float m33 = -((projection.zfar + projection.znear) / (projection.zfar - projection.znear));
		const float m43 = -(2.0f * (projection.zfar * projection.znear) / (projection.zfar - projection.znear));


		return mat4({
			atanf(halfFovX), .0f, .0f, .0f,
			.0f, atanf(halfFovY), .0f, .0f,
			.0f, .0f, m33, m43,
			.0f, .0f, -1.0f, 1.0f
			});
	}

private:
	
	inline static mat4 empty()
	{
		return mat4({
			.0f, .0f, .0f, .0f,
			.0f, .0f, .0f, .0f,
			.0f, .0f, .0f, .0f,
			.0f, .0f, .0f, .0f
			});
	}
};