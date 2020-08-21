#pragma once
#include "Image.h"


class ImageLoader
{
public:

	static Image loadImage(const char *path);
	static void freeImage(Image &image);

private:


	ImageLoader() = delete;
};

