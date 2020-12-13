#pragma once
#include <vector>
#include "Triangle.h"
#include "StrongTypedef.h"
#include "vec.h"
#include "mat.h"
#include "Framebuffer.h"
#include "color.h"
#include "Image.h"
#include "Utilities.h"
#include "AABB.h"

namespace gl
{
	using Model = StrongTypedef<std::vector<Triangle>, struct ModelId>;

	struct PipelineInfo
	{
		FrameBuffer<color> &colorImage;
		FrameBuffer<float> &depthImage;
		const Image &texture;
		mat4x4 model, view, projection;
	};

	class Rasterizer
	{
	public:
		static void draw(const Model &model, const PipelineInfo &pipeline)
		{
			if(pipeline.colorImage.clearOnFirstUse)
			{
				pipeline.colorImage.clear();
			}

			if (pipeline.depthImage.clearOnFirstUse)
			{
				pipeline.depthImage.clear();
			}

			for (const auto &triangle : model.get())
			{
				drawTriangle(triangle, pipeline);
			}
		}

	private:

		static void rasterize(size_t x, size_t y, Triangle& triangle, std::array<float, 3> vertexWs, const PipelineInfo &pipeline)
		{
			const auto barycentricCoords =
				triangle.calculate2DBarycentricCoords(
					vec2(static_cast<float>(x), static_cast<float>(y)),
					vertexWs);

			//if we're inside the triangle, draw it
			if (barycentricCoords.areDegenerate()) return;

			const float zValue = barycentricCoords.weigh(
				triangle.vertices[0].position.z(),
				triangle.vertices[1].position.z(),
				triangle.vertices[2].position.z()
			);

			//depth test
			float &depth = pipeline.depthImage.at(x, y);
			if (depth > zValue)
			{
				depth = zValue;

				const vec3 vertexCol = barycentricCoords.weigh(
					triangle.vertices[0].color,
					triangle.vertices[1].color,
					triangle.vertices[2].color
				);

				const vec3 normal = barycentricCoords.weigh(
					triangle.vertices[0].normal,
					triangle.vertices[1].normal,
					triangle.vertices[2].normal
				).normalized();


				const float u = barycentricCoords.weigh(
					triangle.vertices[0].u,
					triangle.vertices[1].u,
					triangle.vertices[2].u
				);

				const float v = barycentricCoords.weigh(
					triangle.vertices[0].v,
					triangle.vertices[1].v,
					triangle.vertices[2].v
				);


				const vec3 textureCol = pipeline.texture.atUV(u, v);

				const float lambertian = vec3::dot(normal, vec3(.0f, .0f, 1.0f));
				const vec3 col = (textureCol * vertexCol * lambertian).saturate();

				pipeline.colorImage.at(x, y) =
				{
					.b = static_cast<uint8_t>(col.b() * 255.0f),
					.g = static_cast<uint8_t>(col.g() * 255.0f),
					.r = static_cast<uint8_t>(col.r() * 255.0f),
					.a = 255
				};
			}
		}

		static void drawTriangle(Triangle triangle, const PipelineInfo &pipeline)
		{
			//vertex shader
			mat4x4 viewportMat = mat4x4::viewport({
				.x = 0,
				.y = 0,
				.width = pipeline.colorImage.width,
				.height = pipeline.colorImage.height,
			});

			std::array<float,3> vertexWs{};
			for (size_t vertexIndex = 0; vertexIndex < 3U; vertexIndex++)
			{
				Triangle::Vertex &vertex = triangle.vertices[vertexIndex];

				vec4 position = vec4::fromPoint(vertex.position);
				position = viewportMat* pipeline.projection * pipeline.view * pipeline.model * position;

				vertexWs[vertexIndex] = position.w();
				if (!isApproximatively(position.w(), .0f, .00001f)) position /= position.w();
				
				vertex.position = position.xyz();

				vec4 normal = vec4::fromDirection(vertex.normal);
				normal = (pipeline.model).inversed().transposed() * normal;
				vertex.normal = normal.xyz();
			}

			//backface culling
			if (vec3::dot(triangle.calculateFaceNormal(), vec3(.0f, .0f, 1.0f)) < 0)
			{
				return;
			}

			AABB2 imageBounds = pipeline.colorImage.bounds;
			imageBounds.max.x()--;
			imageBounds.max.y()--;
			const AABB2 triangleAABB = triangle.calculateAABB2().boundInto(imageBounds);

			if (!triangleAABB.hasArea())
			{
				//triangle is outside of the bounds of the screen
				return;
			}

			for (uint32_t y = static_cast<uint32_t>(triangleAABB.min.y()); y <= static_cast<uint32_t>(triangleAABB.max.y()); y++)
			for (uint32_t x = static_cast<uint32_t>(triangleAABB.min.x()); x <= static_cast<uint32_t>(triangleAABB.max.x()); x++)
			{
				rasterize(x, y, triangle, vertexWs, pipeline);
			}
		}
	};

}