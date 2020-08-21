#pragma once
#include <vector>
#include "Triangle.h"
#include "StrongTypedef.h"
#include "vec4.h"
#include "mat4.h"
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
		mat4 model, view, projection;
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
		static void drawTriangle(const Triangle &triangle, const PipelineInfo &pipeline)
		{
			//vertex shader
			Triangle transformedTriangle = triangle;

			mat4::Viewport viewport =
			{
				.x = 0,
				.y = 0,
				.width = pipeline.colorImage.width,
				.height = pipeline.colorImage.height,
			};

			mat4 viewportMat = mat4::viewport(viewport);

			for (size_t vertexIndex = 0; vertexIndex < 3; vertexIndex++)
			{
				Triangle::Vertex &vertex = transformedTriangle.vertices[vertexIndex];

				vec4 position = vec4::fromPoint(vertex.position);
				position = pipeline.projection * pipeline.view * pipeline.model * position;
				if (!isApproximatively(position.w, .0f, .00001f)) position = position / position.w;
				position = viewportMat * position;
				vertex.position = position.xyz();

				vec4 normal = vec4::fromDirection(vertex.normal);
				normal = mat4::inversed(mat4::transposed(pipeline.view * pipeline.model)) * normal;
				vertex.normal = normal.xyz();
			}

			//backface culling
			if (vec3::dot(transformedTriangle.calculateFaceNormal(), vec3(.0f, .0f, 1.0f)) < 0)
			{
				return;
			}

			const AABB &imageBounds = pipeline.colorImage.bounds;
			const AABB triangleAABB = transformedTriangle.calculateAABB().boundInto(imageBounds);

			if (triangleAABB.isPoint())
			{
				//triangle is outside of the bounds of the screen, we can return since we're not going to draw anything
				return;
			}

			//for every pixel in the aabb, rasterize
			for (uint32_t y = static_cast<uint32_t>(triangleAABB.min.y); y <= static_cast<uint32_t>(triangleAABB.max.y); y++)
				for (uint32_t x = static_cast<uint32_t>(triangleAABB.min.x); x <= static_cast<uint32_t>(triangleAABB.max.x); x++)
				{
					const BarycentricCoordinates barycentricCoords = transformedTriangle.calculate2DBarycentricCoords(vec3(static_cast<float>(x), static_cast<float>(y), .0f));
					//if we're inside the triangle, draw it
					if (barycentricCoords.areDegenerate()) continue;

					const float zValue = barycentricCoords.weigh(
						transformedTriangle.vertices[0].position.z,
						transformedTriangle.vertices[1].position.z,
						transformedTriangle.vertices[2].position.z
					);

					//depth test
					float &depth = pipeline.depthImage.at(x, y);
					if (depth < zValue)
					{
						depth = zValue;

						const vec3 vertexCol = barycentricCoords.weigh(
							transformedTriangle.vertices[0].color,
							transformedTriangle.vertices[1].color,
							transformedTriangle.vertices[2].color
						);

						const vec3 normal = barycentricCoords.weigh(
							transformedTriangle.vertices[0].normal,
							transformedTriangle.vertices[1].normal,
							transformedTriangle.vertices[2].normal
						).normalized();


						const float u = barycentricCoords.weigh(
							transformedTriangle.vertices[0].u,
							transformedTriangle.vertices[1].u,
							transformedTriangle.vertices[2].u
						);

						const float v = barycentricCoords.weigh(
							transformedTriangle.vertices[0].v,
							transformedTriangle.vertices[1].v,
							transformedTriangle.vertices[2].v
						);


						const vec3 textureCol = pipeline.texture.atUV(u, v);

						const float lambertian = vec3::dot(normal, vec3(.0f, .0f, 1.0f));
						const vec3 col = vec3::saturate(textureCol * vertexCol * lambertian);

						pipeline.colorImage.at(x, y) = { static_cast<uint8_t>(col.b * 255.0f),static_cast<uint8_t>(col.g * 255.0f),static_cast<uint8_t>(col.r * 255.0f), 255 };
					}
				}
		}
	};

}