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

void shadowMapPass(const Time &time)
{
	shadowMap.clear();

	MVP mvp = getShadowMapMVP(time);

	auto vertexShader = [&mvp](Triangle::Vertex vertex)
	{
		vec4 &position = vertex.position;
		position = mvp.projection * mvp.view * mvp.model * position;

		vec4 normal = vec4::fromDirection(vertex.normal);
		normal = mvp.model.inversed().transposed() * normal;
		vertex.normal = normal.xyz();

		return vertex;
	};

	auto fragmentShader = []([[maybe_unused]] const Triangle::Vertex &vertex)
	{
		return vec4();
	};

	auto drawInfo = gl::DrawInfo{ colorImage, vertexShader, fragmentShader, &shadowMap };

	gl::Rasterizer::drawTriangles(handle, drawInfo);
}


void colorPass(const Time& time)
{
	colorImage.clear();
	depthImage.clear();

	const MVP mvp = getMVP(time);

	auto vertexShader = [&mvp](Triangle::Vertex vertex)
	{
		vec4 &position = vertex.position;
		position = mvp.projection * mvp.view * mvp.model * position;

		vec4 normal = vec4::fromDirection(vertex.normal);
		normal = mvp.model.inversed().transposed() * normal;
		vertex.normal = normal.xyz();

		return vertex;
	};

	const MVP shadowMapMVP = getShadowMapMVP(time);

	auto fragmentShader = [&](const Triangle::Vertex &vertex)
	{
		const vec3 textureCol = texture.atUV(vertex.u, vertex.v);

		const float lambertian = vec3::dot(vertex.normal, lightDirection);
		const vec3 col = (textureCol * vertex.color * lambertian).saturate();



		return vec4::fromPoint(col);
	};

	auto drawInfo = gl::DrawInfo{ colorImage, vertexShader, fragmentShader, &depthImage };

	gl::Rasterizer::drawTriangles(handle, drawInfo);
}


int main()
{
	RenderToWindow window(width, height, "rasterizer!!!");

	CoreLoop::run([&](const Time &time)
	{
		shadowMapPass(time);
		colorPass(time);
		window.updateImage(colorImage.data);
	});

	return 0;
}