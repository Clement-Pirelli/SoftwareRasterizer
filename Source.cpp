#include "RenderToWindow/RenderToWindow.h"
#include <cstring>
#include <utility>
#include <limits>
#include "AABB.h"
#include "vec3.h"
#include <vector>
#include "Framebuffer.h"
#include "CoreLoop.h"
#include "Triangle.h"
#include "ModelLoader.h"
#include "ImageLoader.h"
#include "vec4.h"
#include "mat4.h"

struct PipelineInfo
{
	FrameBuffer<color> &colorImage;
	FrameBuffer<float> &depthImage;
	const Image &texture;
};

float isApproximatively(float a, float b, float approx)
{
	return (a + approx) > b && (a - approx) < b;
}

template<class T>
T weighByBarycentricCoords(const T &firstValue, const T &secondValue, const T &thirdValue, const vec3 &barycentricCoords)
{
	return firstValue * barycentricCoords.x + secondValue * barycentricCoords.y + thirdValue * barycentricCoords.z;
}

void drawTriangle(const Triangle &triangle, PipelineInfo pipeline)
{
	const AABB &imageBounds = pipeline.colorImage.bounds;
	const AABB aabb = triangle.calculateAABB().boundInto(imageBounds);

	if (isApproximatively(aabb.min.x, aabb.max.x, std::numeric_limits<float>::epsilon()) &&
		isApproximatively(aabb.min.y, aabb.max.y, std::numeric_limits<float>::epsilon()))
	{
		//triangle is outsite of the bounds of the screen, we can return since we're not going to draw anything
		return;
	}

	//backface culling
	if(vec3::dot(triangle.calculateFaceNormal(), vec3(.0f,.0f,1.0f)) < 0)
	{
		return;
	}

	for(size_t y = aabb.min.y; y < aabb.max.y; y++)
	for (size_t x = aabb.min.x; x < aabb.max.x; x++)
	{
		vec3 barycentricCoords = triangle.calculate2DBarycentricCoords(vec3(static_cast<float>(x),static_cast<float>(y),.0f));
		if (barycentricCoords.x < 0.0f || barycentricCoords.y < 0.0f || barycentricCoords.z < 0.0f) continue;

		float zValue = weighByBarycentricCoords(triangle.v1.z, triangle.v2.z, triangle.v3.z, barycentricCoords);

		float &depth = pipeline.depthImage.at(x, y);
		if(depth < zValue)
		{
			depth = zValue;

			const vec3 vertexCol = weighByBarycentricCoords(
				triangle.c1,
				triangle.c2,
				triangle.c3,
				barycentricCoords);

			const vec3 normal = weighByBarycentricCoords(
				triangle.n1,
				triangle.n2,
				triangle.n3,
				barycentricCoords).normalized();

			const float u = weighByBarycentricCoords(triangle.us.x, triangle.us.y, triangle.us.z, barycentricCoords);
			const float v = weighByBarycentricCoords(triangle.vs.x, triangle.vs.y, triangle.vs.z, barycentricCoords);
			const vec3 textureCol = pipeline.texture.atUV(u, v);

			float lambertian = vec3::dot(normal, vec3(.0f, .0f, 1.0f));
			vec3 col = textureCol * vertexCol * lambertian;
			col = col.clampedBy(vec3(.0f,.0f,.0f), vec3(1.0f,1.0f,1.0f));

			pipeline.colorImage.at(x, y) = { static_cast<uint8_t>(col.b * 255.0f),static_cast<uint8_t>(col.g * 255.0f),static_cast<uint8_t>(col.r * 255.0f), 255 };
		}
	}
}

int main()
{
	mat4 mat = mat4::inversed(mat4::transposed(mat4::scale(vec3(1.0f, 2.0f, 2.0f))));


	constexpr size_t width = 1000u, height = 1000u;
	auto colorImage = FrameBuffer<color>(width, height, {0,0,0,255});
	auto depthImage = FrameBuffer<float>(width, height, .0f);

	RenderToWindow window(width, height, "rasterizer!!!");
	
	std::vector<Triangle> triangles = ModelLoader::loadModel("assets/head.obj");

	Image texture = ImageLoader::loadImage("assets/head_diffuse.png");

	//CoreLoop().run([&tri, &colorImage, &depthImage, &window]() 
	//{
		colorImage.clear({0,0,0,255});
		depthImage.clear(.0f);

		PipelineInfo pipeline
		{
			.colorImage = colorImage,
			.depthImage = depthImage,
			.texture = texture
		};

		for(const auto &triangle : triangles)
		{
			drawTriangle(triangle, pipeline);
		}
		window.updateImage(colorImage.data);
	//});
		window.handleMessagesBlocking();

	ImageLoader::freeImage(texture);
	



	return 0;
}