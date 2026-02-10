#include <utils/Texture.hpp>
#include <stb_image.h>
#include <iostream>

const std::string Texture::DEFAULT_TEXTURE_PATH = "assets/textures/default_texture.png";

Texture::Texture() : Texture(DEFAULT_TEXTURE_PATH) { };

Texture::Texture(const std::string& filePath)
{
	//stbi_set_flip_vertically_on_load(true);	// !!! Could also be set in Cubemap
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &num_channels, 4);

	if (!data) {
		std::cout << "WARNING::Texture::ctor: Failed to load texture " << filePath << " - " << stbi_failure_reason() << std::endl;
		std::cout << "Loading default texture." << std::endl << std::endl;
		
		data = stbi_load(DEFAULT_TEXTURE_PATH.c_str(), &width, &height, &num_channels, 4);
		if (!data) std::cout << "ERROR::Texture::ctor: Failed to load default texture - " << stbi_failure_reason() << std::endl << std::endl;
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
}

Texture::Texture(Texture&& other) noexcept
{
	_id = other._id;
	width = other.width;
	height = other.height;
	num_channels = other.num_channels;

	other._id = 0;
	other.width = 0;
	other.height = 0;
	other.num_channels = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other) {	// Forbid self-move
		cleanup();			// Clean up this before moving data to it, so as to not leave resources dangling
		_id = other._id;
		width = other.width;
		height = other.height;
		num_channels = other.num_channels;

		other._id = 0;
		other.width = 0;
		other.height = 0;
		other.num_channels = 0;
	}
	return *this;
}

Texture::~Texture() { cleanup(); }

void Texture::cleanup() { glDeleteTextures(1, &_id); }

GLuint Texture::get_ID() const { return _id; }