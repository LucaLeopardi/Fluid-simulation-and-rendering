#pragma once

#include <string>
#include <glad/glad.h>

class Texture
{
private:

	static const std::string TEXTURE_PATH;
	GLuint _id = 0;

public:

	int width = 0;
	int height = 0;
	int num_channels = 0;

	Texture();

	Texture(const std::string& filePath);

	// Copy constructor/assignment are forbidden: we don't want multiple instances pointing to the same OpenGL data, since it can be deleted by them
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;

	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	~Texture();

	void cleanup();

	GLuint get_ID() const;
};