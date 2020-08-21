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
	mat4 model, view, projection;
};

void drawTriangle(const Triangle &triangle, const PipelineInfo &pipeline)
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
	for(int y = static_cast<int>(triangleAABB.min.y); y < static_cast<int>(triangleAABB.max.y); y++)
	for (int x = static_cast<int>(triangleAABB.min.x); x < static_cast<int>(triangleAABB.max.x); x++)
	{
		const BarycentricCoordinates barycentricCoords = transformedTriangle.calculate2DBarycentricCoords(vec3(static_cast<float>(x),static_cast<float>(y),.0f));
		//if we're inside the triangle, draw it
		if (barycentricCoords.areDegenerate()) continue;		

		const float zValue = barycentricCoords.weigh(
			transformedTriangle.vertices[0].position.z, 
			transformedTriangle.vertices[1].position.z, 
			transformedTriangle.vertices[2].position.z
		);

		//depth test
		float &depth = pipeline.depthImage.at(x, y);
		if(depth < zValue)
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

int main()
{
	constexpr size_t width = 1000u, height = 1000u;
	auto colorImage = FrameBuffer<color>(width, height, {0,0,0,255});
	auto depthImage = FrameBuffer<float>(width, height, .0f);

	RenderToWindow window(width, height, "rasterizer!!!");
	
	std::vector<Triangle> triangles = ModelLoader::loadModel("assets/head.obj");

	Image texture = ImageLoader::loadImage("assets/head_diffuse.png");

	CoreLoop().run([&](const Time &time) 
	{
		colorImage.clear({0,0,0,255});
		depthImage.clear(.0f);

		PipelineInfo pipeline
		{
			.colorImage = colorImage,
			.depthImage = depthImage,
			.texture = texture,
			.model = mat4::rotateY(time.asSeconds()),
			.view = mat4::identity(),
			.projection = mat4::identity()
		};

		for(const auto &triangle : triangles)
		{
			drawTriangle(triangle, pipeline);
		}
		window.updateImage(colorImage.data);
	});

	ImageLoader::freeImage(texture);
	

	return 0;
}