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
#include <unordered_map>

namespace gl
{
	using Model = StrongTypedef<std::vector<Triangle>, struct ModelId>;
	using BufferHandle = StrongTypedef<uint64_t, struct BufferHandleId>;

	struct PipelineInfo
	{
		FrameBuffer<color> &colorImage;
		FrameBuffer<float> &depthImage;
		const Image &texture;
		mat4x4 model, view, projection;
	};

	template<typename T, typename U>
	concept AreSame = requires(T a, U b)
	{
		std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;
		std::is_same_v<std::remove_cv_t<U>, std::remove_cv_t<T>>;
	};

	template<typename T>
	concept Shader = requires(T a)
	{
		std::is_invocable_v<T>;
		//todo: check the in parameter, out parameter?
	};

	template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
	struct DrawInfo
	{
		FrameBuffer<RenderTarget_t> &target;
		Vertex_t vertexShader;
		Fragment_t fragmentShader;
		FrameBuffer<float>* depthBuffer = nullptr;
	};

	template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
	DrawInfo(FrameBuffer<RenderTarget_t> &a, Vertex_t b, Fragment_t c, FrameBuffer<float>*d)->DrawInfo<RenderTarget_t, Vertex_t, Fragment_t>;
	template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
	DrawInfo(FrameBuffer<RenderTarget_t> &a, Vertex_t b, Fragment_t c)->DrawInfo<RenderTarget_t, Vertex_t, Fragment_t>;


	class Rasterizer
	{
	public:

		[[nodiscard]]
		static BufferHandle uploadBuffer(const Model &model)
		{
			static uint64_t nextHandle = 0U;
			nextHandle++;
			buffers[BufferHandle(nextHandle)] = model;
			return nextHandle;
		}

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
		static void drawTriangles(BufferHandle handle, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t>& drawInfo)
		{
			if(buffers.contains(handle))
			{
				const Model &buffer = buffers[handle];
				for (const auto &triangle : buffer.get())
				{
					drawTriangle(triangle, drawInfo);
				}
			}
		}

	private:

		inline static std::unordered_map<BufferHandle, Model> buffers;

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
		static void rasterize(size_t x, size_t y, Triangle& triangle, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t> &drawInfo)
		{
			const std::array<float, 3> vertexWs{ triangle.vertices[0].position.w(), triangle.vertices[1].position.w(), triangle.vertices[2].position.w(), };

			const auto barycentricCoords =
				triangle.calculate2DBarycentricCoords(
					vec2(static_cast<float>(x), static_cast<float>(y)),
					vertexWs);

			//if we're inside the triangle, draw it
			if (barycentricCoords.areDegenerate()) return;

			const vec4 pos = barycentricCoords.weigh(
				triangle.vertices[0].position,
				triangle.vertices[1].position,
				triangle.vertices[2].position
			);
			
			auto depthTest = [&drawInfo, x, y](float z)
			{
				if (drawInfo.depthBuffer != nullptr)
				{
					float &depth = drawInfo.depthBuffer->at(x, y);
					if (depth <= z)
					{
						return false;
					}
					depth = z;
				}
				return true;
			};
			
			if (depthTest(pos.z()))
			{
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

				const Triangle::Vertex weighedVertex
				{
					.position = pos,
					.color = vertexCol,
					.u = u,
					.v = v,
					.normal = normal
				};

				drawInfo.target.at(x, y) = drawInfo.fragmentShader(weighedVertex);
			}
		}

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t>
		static void drawTriangle(Triangle triangle, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t> &drawInfo)
		{
			//vertex shader
			mat4x4 viewportMat = mat4x4::viewport({
				.x = 0,
				.y = 0,
				.width = drawInfo.target.width,
				.height = drawInfo.target.height,
			});

			for (Triangle::Vertex &vertex : triangle.vertices)
			{
				vertex = drawInfo.vertexShader(vertex);
				vec4 &position = vertex.position;
				position = viewportMat * position;
				if (!isApproximatively(position.w(), .0f, .00001f)) position /= position.w();
			}

			//backface culling
			if (vec3::dot(triangle.calculateFaceNormal(), vec3(.0f, .0f, 1.0f)) < 0)
			{
				return;
			}

			AABB2 imageBounds = drawInfo.target.bounds;
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
				rasterize(x, y, triangle, drawInfo);
			}
		}
	};

}