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
#include "CommonConcepts.h"

namespace gl
{
	using Model = StrongTypedef<std::vector<Triangle>, struct ModelId>;
	using BufferHandle = StrongTypedef<uint64_t, struct BufferHandleId>;

	template<typename T>
	concept Shader = requires(T a)
	{
		std::is_invocable_v<T>;
		//todo: check the in parameter, out parameter?
	};

	template<typename T>
	concept Attributes = requires(Triangle::BarycentricCoordinates a)
	{
		Same<decltype(T::barycentricInterpolation(a, T{}, T{}, T{})), T>;
	};

	template<Attributes Attributes_t>
	struct VertexReturn
	{
		Triangle::Vertex vertex;
		Attributes_t attributes;
	};

	template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t, Attributes Attributes_t>
	struct DrawInfo
	{
		FrameBuffer<RenderTarget_t> &target;
		Vertex_t vertexShader;
		Fragment_t fragmentShader;
		FrameBuffer<float>* depthBuffer = nullptr;
	};

	template<typename RenderTarget_t, Attributes Attributes_t>
	auto makeDrawInfo(FrameBuffer<RenderTarget_t>&target, auto vertexShader, auto fragmentShader, FrameBuffer<float>* depthBuffer = nullptr)
	{
		return DrawInfo <RenderTarget_t, decltype(vertexShader), decltype(fragmentShader), Attributes_t>
		{
			.target = target,
			.vertexShader = vertexShader,
			.fragmentShader = fragmentShader,
			.depthBuffer = depthBuffer
		};
	}

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

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t, Attributes Attributes_t>
		static void drawTriangles(BufferHandle handle, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t, Attributes_t>& drawInfo)
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

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t, Attributes Attributes_t>
		static void rasterize(size_t x, size_t y, Triangle& triangle, const std::array<Attributes_t, 3>&attributes, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t, Attributes_t> &drawInfo)
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
					float &depth = drawInfo.depthBuffer->atTexel(x, y);
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
				const Triangle::Vertex weighedVertex = Triangle::Vertex::barycentricInterpolation(
					barycentricCoords,
					triangle.vertices[0],
					triangle.vertices[1],
					triangle.vertices[2]
				);

				const Attributes_t weighedAttributes = Attributes_t::barycentricInterpolation(
					barycentricCoords,
					attributes[0],
					attributes[1],
					attributes[2]
				);

				drawInfo.target.atTexel(x, y) = drawInfo.fragmentShader(weighedVertex, weighedAttributes);
			}
		}

		template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t, Attributes Attributes_t>
		static void drawTriangle(Triangle triangle, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t, Attributes_t> &drawInfo)
		{
			//vertex shader
			mat4x4 viewportMat = mat4x4::viewport({
				.x = 0,
				.y = 0,
				.width = drawInfo.target.width,
				.height = drawInfo.target.height,
			});

			std::array<Attributes_t, 3> attributesArray = {};
			for(size_t i = 0; i < 3; i++)
			{
				Triangle::Vertex &vertex = triangle.vertices[i];
				const VertexReturn result = drawInfo.vertexShader(vertex);
				attributesArray[i] = result.attributes;
				vertex = result.vertex;
				vec4 &position = vertex.position;
				position = viewportMat * position;
				if (!isApproximatively(position.w(), .0f, .00001f)) 
				{ 
					position /= position.w();
				};
			}

			//backface culling
			if (vec3::dot(triangle.calculateFaceNormal(), vec3(.0f, .0f, 1.0f)) < 0)
			{
				return;
			}

			AABB2 imageBounds = drawInfo.target.bounds();
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
				rasterize(x, y, triangle, attributesArray, drawInfo);
			}
		}
	};

}