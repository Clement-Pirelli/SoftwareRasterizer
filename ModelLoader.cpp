#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct Vertex
{
	vec3 position;
	vec3 color;
	float u, v;
	vec3 normal;
};

#pragma warning(disable : 6386)
#pragma warning(disable : 26451)

std::vector<Triangle> ModelLoader::loadModel(const char *filePath)
{
	std::string filePathStr = std::string(filePath);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<Vertex> vertices;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath)) {
		throw std::runtime_error(warn + err);
	}

	if (attrib.vertices.size() == 0)
	{
		throw std::runtime_error("Model at " + filePathStr + " has no vertices!");
	}

	if (attrib.texcoords.size() == 0)
	{
		throw std::runtime_error("Model at " + filePathStr + " has no UVs!");
	}

	for (const auto &shape : shapes) {
		for (const auto &index : shape.mesh.indices) {


			Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//vertex.position = vertex.position + vec3(100.0f, 100.0f, .0f);
			vertex.position = vertex.position * 500.0f;
			vertex.position.x += 500.0f;
			vertex.position.y += 500.0f;

			vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
			vertex.v = attrib.texcoords[2 * index.texcoord_index + 1];
			
			vertex.color =
			{
				1.0f,1.0f,1.0f
			};

			vertex.normal = vec3(
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
				);

			vertices.push_back(vertex);
		}
	}
	
	std::vector<Triangle> triangles;
	for(int i = 0; i < vertices.size(); i+=3)
	{
		Triangle triangle
		{
			.v1 = vertices[i].position,
			.v2 = vertices[i + 1].position,
			.v3 = vertices[i + 2].position,

			.c1 = vertices[i].color,
			.c2 = vertices[i + 1].color,
			.c3 = vertices[i + 2].color,

			.us = vec3(vertices[i].u, vertices[i + 1].u, vertices[i + 2].u),
			.vs = vec3(vertices[i].v, vertices[i + 1].v, vertices[i + 2].v),

			.n1 = vertices[i].normal,
			.n2 = vertices[i + 1].normal,
			.n3 = vertices[i + 2].normal,
		};
		triangles.push_back(triangle);
	}

	return triangles;
}