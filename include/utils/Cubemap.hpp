#pragma once

#include <string>
#include <glad/glad.h>

class Cubemap
{
private:

	static const std::string CUBEMAP_PATHS[6];
	GLuint _id = 0;

public:

	int width = 0;
	int height = 0;
	int num_channels = 0;

	Cubemap();

	Cubemap(const std::string (&file_paths)[6]);

	// Copy constructor/assignment are forbidden: we don't want multiple instances pointing to the same OpenGL data, since it can be deleted by them
	Cubemap(const Cubemap& other) = delete;
	Cubemap& operator=(const Cubemap& other) = delete;

	Cubemap(Cubemap&& other) noexcept;
	Cubemap& operator=(Cubemap&& other) noexcept;

	~Cubemap();

	void cleanup();

	GLuint get_ID() const;
};