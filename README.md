# Software Rasterizer

A software rasterizer vaguely based on [tinyrenderer](https://github.com/ssloy/tinyrenderer)

Additionally, a display of my going completely insane, as evidenced by such classics as:

```cpp
template<typename RenderTarget_t, Shader Vertex_t, Shader Fragment_t, Attributes Attributes_t>
static void rasterize(int x, int y, Triangle& triangle, const std::array<Attributes_t, 3>&attributes, DrawInfo<RenderTarget_t, Vertex_t, Fragment_t, Attributes_t> &drawInfo)
```

That being said the resulting API is pretty nice to use. 
For example, this is how I render the shadowmap:

```cpp
void shadowMapPass(const Time &time)
{
	shadowMap.clear();
	depthImage.clear();

	const mat4 mvpMat = getShadowMapMVP(time).calculate();

	auto vertexShader = [&mvpMat](Triangle::Vertex vertex) -> gl::VertexReturn<ShadowPassAttributes>
	{
		vertex.position = mvpMat * vertex.position;
		return { vertex };
	};

	auto fragmentShader = [](const Triangle::Vertex &v, ShadowPassAttributes)
	{
		return v.position.z();
	};

	auto drawInfo = gl::makeDrawInfo<float, ShadowPassAttributes>(shadowMap, vertexShader, fragmentShader, &depthImage);

	gl::Rasterizer::drawTriangles(handle, drawInfo);
}
```
