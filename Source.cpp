#include "RenderToWindow/RenderToWindow.h"
#include "AABB.h"
#include "Framebuffer.h"
#include "ModelLoader.h"
#include "Image.h"
#include "vec.h"
#include "mat.h"
#include "CoreLoop.h"
#include "Triangle.h"
#include "GraphicsLibrary.h"
#include "BMPWriter.h"

#include <cstring>
#include <utility>
#include <limits>
#include <vector>
#include <span>
#include <algorithm>

constexpr size_t width = 500u, height = 500u;

gl::FrameBuffer<vec4> colorImage = gl::FrameBuffer<vec4>({
	.width = width,
	.height = height,
	.clearValue = {0,0,0,255}
	});

gl::FrameBuffer<float> depthImage = gl::FrameBuffer<float>({
	.width = width,
	.height = height,
	.clearValue = 1000000000.0f
	});

gl::FrameBuffer<float> shadowMap = gl::FrameBuffer<float>({
	.width = width,
	.height = height,
	.clearValue = 1000000000.0f
	});

const gl::BufferHandle handle = gl::Rasterizer::uploadBuffer(ModelLoader::loadModel("assets/head.obj"));

const Image texture = Image("assets/head_diffuse.png");

const vec3 lightDirection = vec3(1.0f, 1.0f, 1.0f).normalized();

struct MVP
{
	mat4x4 model;
	mat4x4 view;
	mat4x4 projection;
};

MVP getMVP(const Time& time)
{
	return
	{
		.model = mat3x3::rotatedY(time.asSeconds()).expandTo<4>(),
		.view = mat4x4::translate(vec3(.0f, .0f, -2.0f)),
		.projection = mat4x4::perspective(
		{
			.fovX = 50.0f * 3.14159265359f / 180.0f,
			.aspectRatio = width / static_cast<float>(height),
			.zfar = 10000.0f,
			.znear = .1f
		})
	};
}

MVP getShadowMapMVP(const Time &time)
{
	return
	{
		.model = mat3x3::rotatedY(time.asSeconds()).expandTo<4>(),
		.view = mat4x4::lookAt({
			.eye = lightDirection * -2.0f, 
			.target = vec3(), 
			.up = vec3(.0f,1.0f,.0f)}),
		.projection = mat4x4::orthographic({
			.right = 1,
			.left = -1,
			.top = 1,
			.bottom = -1,
			.far = 5.0f,
			.near = .2f})
	};
}

struct ShadowPassAttributes
{
	static ShadowPassAttributes barycentricInterpolation(Triangle::BarycentricCoordinates, ShadowPassAttributes, ShadowPassAttributes, ShadowPassAttributes)
	{
		return {};
	}
};

void shadowMapPass(const Time &time)
{
	shadowMap.clear();

	MVP mvp = getShadowMapMVP(time);

	auto vertexShader = [&mvp](Triangle::Vertex vertex) -> gl::VertexReturn<ShadowPassAttributes>
	{
		vec4 &position = vertex.position;
		position = mvp.projection * mvp.view * mvp.model * position;
		return { vertex };
	};

	auto fragmentShader = [](const Triangle::Vertex &v, ShadowPassAttributes)
	{
		return v.position.z();
	};

	auto drawInfo = gl::makeDrawInfo<float, ShadowPassAttributes>(depthImage, vertexShader, fragmentShader, &shadowMap);

	gl::Rasterizer::drawTriangles(handle, drawInfo);
}

struct ColorPassAttributes
{
	vec4 lightSpacePosition;
	static ColorPassAttributes barycentricInterpolation(Triangle::BarycentricCoordinates coords, ColorPassAttributes a, ColorPassAttributes b, ColorPassAttributes c)
	{
		return { coords.weigh(a.lightSpacePosition, b.lightSpacePosition, c.lightSpacePosition) };
	}
};

void colorPass(const Time& time)
{
	colorImage.clear();
	depthImage.clear();

	const MVP mvp = getMVP(time);
	const MVP shadowMapMVP = getShadowMapMVP(time);

	auto vertexShader = [&mvp, &shadowMapMVP](Triangle::Vertex vertex) -> gl::VertexReturn<ColorPassAttributes>
	{
		const vec4 normal = mvp.model.inversed().transposed() * vec4::fromDirection(vertex.normal);
		vertex.normal = normal.xyz();

		const vec4 lightSpacePosition = shadowMapMVP.projection * shadowMapMVP.view * mvp.model * vertex.position + normal*.01f;

		vertex.position = mvp.projection * mvp.view * mvp.model * vertex.position;

		return { vertex, { lightSpacePosition } };
	};

	auto fragmentShader = [&](const Triangle::Vertex &vertex, ColorPassAttributes attributes)
	{
		const vec3 textureCol = texture.atUV(vertex.u, vertex.v);

		//const float lambertian = vec3::dot(vertex.normal, lightDirection);
		const vec3 col = (textureCol * vertex.color /** lambertian*/).saturate();

		const vec3 lightSpaceProjected = (attributes.lightSpacePosition.xyz() / attributes.lightSpacePosition.w())*vec3(.5f, .5f, 1.0f) + vec3(.5f, .5f, .0f);

		const float closestDepth = shadowMap.atUV(lightSpaceProjected.x(), lightSpaceProjected.y(), sampling::SamplerMode::Bilinear);
		const float currentDepth = lightSpaceProjected.z();
		const float shadow = currentDepth > closestDepth ? 1.0f : 0.5f;

		return vec4::fromPoint(col * shadow);
	};

	auto drawInfo = gl::makeDrawInfo<vec4, ColorPassAttributes>(colorImage, vertexShader, fragmentShader, &depthImage);

	gl::Rasterizer::drawTriangles(handle, drawInfo);
}

void writeToBMP(std::span<float> span, const char* name)
{
	std::vector<bmp::color> spanAsColors(span.size());
	for (size_t i = 0; i < spanAsColors.size(); i++)
	{
		const float in = span[i];
		const unsigned char value = static_cast<unsigned char>(255.9f * in);
		spanAsColors[i] = bmp::color{ value, value , value, 0xff };
	}

	const bmp::writeInfo writeInfo =
	{
		.path = name,
		.xPixelCount = width,
		.yPixelCount = height,
		.contents = spanAsColors.data()
	};
	bmp::write(writeInfo);
}

void writeToBMP(std::span<vec4> span, const char *name)
{
	std::vector<bmp::color> spanAsColors(span.size());	

	for(size_t i = 0; i < spanAsColors.size(); i++)
	{
		constexpr float toByte = 255.9f;
		const vec4 in = span[i];
		spanAsColors[i] = bmp::color{ (uint8_t)(in.b() * toByte), (uint8_t)(in.g() * toByte) , (uint8_t)(in.r() * toByte), 0xff };
	}

	const bmp::writeInfo writeInfo =
	{
		.path = name,
		.xPixelCount = width,
		.yPixelCount = height,
		.contents = spanAsColors.data()
	};
	bmp::write(writeInfo);
}

int main()
{
	RenderToWindow window(width, height, "rasterizer!!!");

	CoreLoop::run([&](const Time &time, const Input &input)
	{
		const bool screenshot = input[input::VirtualKeys::Space] == input::InputState::Pressed;

		shadowMapPass(time);

		if(screenshot)
		{
			const std::span<float> shadowmapData = std::span<float>(shadowMap.data, shadowMap.height * shadowMap.width);
			writeToBMP(shadowmapData, "shadowmap.bmp");
		}

		colorPass(time);

		if(screenshot)
		{
			const std::span<vec4> colorImageData = std::span<vec4>(colorImage.data, colorImage.height * colorImage.width);
			writeToBMP(colorImageData, "color.bmp");
			const std::span<float> depthImageData = std::span<float>(depthImage.data, depthImage.height * depthImage.width);
			writeToBMP(depthImageData, "depth.bmp");
		}

		window.updateImage(colorImage.data);
	});

	return 0;
}