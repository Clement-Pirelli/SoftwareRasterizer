#include "RenderToWindow/RenderToWindow.h"
#include <cstring>
#include <utility>
#include <limits>
#include <vector>
#include "AABB.h"
#include "Framebuffer.h"
#include "ModelLoader.h"
#include "ImageLoader.h"
#include "vec.h"
#include "mat.h"
#include "CoreLoop.h"
#include "Triangle.h"
#include "GraphicsLibrary.h"
#include <iostream>

template<size_t N>
std::ostream &operator<<(std::ostream &os, const squaremat<N> &mat)
{
	for (size_t y = 0; y < N; y++)
	{
		os << "|\t";
		for (size_t x = 0; x < N; x++)
		{
			os << mat.at(x, y) << "\t|\t";
		}
		os << '\n';
	}
	
	return os;
}

int main()
{
	mat4x4 m = mat4x4::identity();
	std::cout << "identity:\n";
	std::cout << m;
	m = m * mat4x4::translate(vec3(1.0f, 2.0f, 1.0f));
	std::cout << "translate (1.0f, 2.0f, 1.0f):\n";
	std::cout << m;
	m = m * mat3x3::scale(vec3(20.0f, 3.0f, .5f)).expandTo<4>();
	std::cout << "scale (20.0f,3.0f,.5f):\n";
	std::cout << m;
	[[maybe_unused]] float det = m.calculateDeterminant();

	mat4x4 mAdjugate = m.calculateAdjugate();
	std::cout << "Adjugate:\n";
	std::cout << mAdjugate;

	mat4x4 mInversed = m.inversed();
	std::cout << "Inversed:\n";
	std::cout << mInversed;

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
		.clearValue = 1000000000.0f,
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
			.model = mat3x3::rotatedY(time.asSeconds()).expandTo<4>(),
			.view = mat4x4::translate(vec3(.0f,.0f, -2.0f)),
			.projection = mat4x4::perspective(
				mat4x4::PerspectiveProjection
				{
					.fovX = 50.0f * 3.14159265359f/180.0f,
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