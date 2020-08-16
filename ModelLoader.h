#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Triangle.h"

class ModelLoader
{
public:

	static std::vector<Triangle> loadModel(const char *filePath);

private:
	ModelLoader() = delete;
};