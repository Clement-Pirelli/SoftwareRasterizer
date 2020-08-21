#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


#pragma warning(disable : 6386)
#pragma warning(disable : 26451)

std::vector<Triangle> ModelLoader::loadModel(const char *filePath)
{
	std::string filePathStr = std::string(filePath);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<Triangle::Vertex> vertices;
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


			Triangle::Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

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
	
	std::vector<Triangle> triangles = {};
	triangles.reserve(vertices.size() / 3);

	for(int i = 0; i < vertices.size(); i+=3)
	{
		const Triangle triangle
		{
			.vertices =
			{
				vertices[i],
				vertices[i + 1],
				vertices[i + 2]
			}
		};

		triangles.push_back(triangle);
	}

	return triangles;
}