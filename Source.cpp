#include "RenderToWindow/RenderToWindow.h"
#include <cstring>
#include <utility>
#include <limits>
#include <vector>
#include "AABB.h"
#include "Framebuffer.h"
#include "ModelLoader.h"
#include "Image.h"
#include "vec.h"
#include "mat.h"
#include "CoreLoop.h"
#include "Triangle.h"
#include "GraphicsLibrary.h"

int main()
{
	constexpr size_t width = 1000u, height = 1000u;

	gl::FrameBuffer<vec4>::CreateInfo colorFrameBufferInfo
	{
		.width = width,
		.height = height,
		.clearValue = {0,0,0,255}
	};
	auto colorImage = gl::FrameBuffer<vec4>(colorFrameBufferInfo);

	gl::FrameBuffer<float>::CreateInfo depthFrameBufferInfo
	{
		.width = width,
		.height = height,
		.clearValue = 1000000000.0f
	};
	auto depthImage = gl::FrameBuffer<float>(depthFrameBufferInfo);

	RenderToWindow window(width, height, "rasterizer!!!");

	gl::BufferHandle handle = gl::Rasterizer::uploadBuffer(ModelLoader::loadModel("assets/head.obj"));

	Image texture("assets/head_diffuse.png");

	CoreLoop::run([&](const Time &time)
	{
		colorImage.clear();
		depthImage.clear();

		mat4x4 model = mat3x3::rotatedY(time.asSeconds()).expandTo<4>();
		mat4x4 view = mat4x4::translate(vec3(.0f, .0f, -2.0f));
		mat4x4 projection = mat4x4::perspective(
			{
				.fovX = 50.0f * 3.14159265359f / 180.0f,
				.aspectRatio = width / static_cast<float>(height),
				.zfar = 10000.0f,
				.znear = .1f
			});

		auto vertexShader = [&model, &view, &projection](Triangle::Vertex vertex)
		{
			vec4 &position = vertex.position;
			position = projection * view * model * position;

			vec4 normal = vec4::fromDirection(vertex.normal);
			normal = model.inversed().transposed() * normal;
			vertex.normal = normal.xyz();

			return vertex;
		};

		auto fragmentShader = [&texture](const Triangle::Vertex &vertex)
		{
			const vec3 textureCol = texture.atUV(vertex.u, vertex.v);

			const float lambertian = vec3::dot(vertex.normal, vec3(.0f, .0f, 1.0f));
			const vec3 col = (textureCol * vertex.color * lambertian).saturate();
			return vec4::fromPoint(col);
		};

		auto depthTest = [&depthImage](size_t x, size_t y, float z)
		{
			float &depth = depthImage.at(x, y);
			if (depth > z)
			{
				depth = z;
				return true;
			}
			else
			{
				return false;
			}
		};

		auto drawInfo = gl::DrawInfo{ colorImage, vertexShader, fragmentShader, depthTest };

		gl::Rasterizer::drawTriangles(handle, drawInfo);

		window.updateImage(colorImage.data);
	});

	return 0;
}