#include "RenderToWindow/RenderToWindow.h"
#include <cstring>
#include <utility>
#include <limits>
#include <vector>
#include "AABB.h"
#include "vec3.h"
#include "Framebuffer.h"
#include "ModelLoader.h"
#include "ImageLoader.h"
#include "vec4.h"
#include "mat4.h"
#include "CoreLoop.h"
#include "Triangle.h"
#include "GraphicsLibrary.h"

int main()
{
	constexpr size_t width = 1000u, height = 1000u;


	gl::FrameBuffer<color>::CreateInfo colorFrameBufferInfo
	{
		.width = width,
		.height = height,
		.clearValue = {0,0,0,255},
		.clearOnFirstUse = true
	};
	auto colorImage = gl::FrameBuffer<color>(colorFrameBufferInfo);

	gl::FrameBuffer<float>::CreateInfo depthFrameBufferInfo
	{
		.width = width,
		.height = height,
		.clearValue = .0f,
		.clearOnFirstUse = true
	};
	auto depthImage = gl::FrameBuffer<float>(depthFrameBufferInfo);

	RenderToWindow window(width, height, "rasterizer!!!");
	
	gl::Model model = ModelLoader::loadModel("assets/head.obj");

	Image texture = ImageLoader::loadImage("assets/head_diffuse.png");

	CoreLoop::run([&](const Time &time)
	{
		gl::PipelineInfo pipeline
		{
			.colorImage = colorImage,
			.depthImage = depthImage,
			.texture = texture,
			.model = mat4::rotateY(time.asSeconds()),
			.view = mat4::translate(vec3(.0f,.0f, -2.0f)),
			.projection = mat4::perspective(
				mat4::PerspectiveProjection
				{
					.fovX = 2.0f * 3.14159265359f,
					.aspectRatio = width/static_cast<float>(height),
					.zfar = time.asSeconds(),
					.znear = .1f
				}
			)
		};

		gl::Rasterizer::draw(model, pipeline);

		window.updateImage(colorImage.data);
	});

	ImageLoader::freeImage(texture);

	return 0;
}