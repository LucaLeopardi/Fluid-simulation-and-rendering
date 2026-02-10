#include <utils/Cubemap.hpp>
#include <stb_image.h>
#include <iostream>

const std::string Cubemap::CUBEMAP_PATHS[6] = {
	"assets/textures/default_texture.png",
	"assets/textures/default_texture.png",
	"assets/textures/default_texture.png",
	"assets/textures/default_texture.png",
	"assets/textures/default_texture.png",
	"assets/textures/default_texture.png",
};

Cubemap::Cubemap() : Cubemap(CUBEMAP_PATHS) { };

Cubemap::Cubemap(const std::string (&filePaths)[6])
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _id);

	//stbi_set_flip_vertically_on_load(true);	// !!! Could also be set in Texture
	for (int i (0); i < 6; ++i) {
		// Cubemap width, height and num_channels will result as the last texture's, meaning they refer to face width, height and num_channels. And as it's a cube, it's reasonable to assume they're the same for all faces.
		// UNLESS a load fails and the default texture is loaded for just some of the faces. In which case there's a bigger problem than Cubemap width, height and num_channels not being accurate (also because they're not currently used beyond initialization). So I'd say it's ok.
		unsigned char* data = stbi_load(filePaths[i].c_str(), &width, &height, &num_channels, 4);

		if (!data) {
			std::cout << "WARNING::Cubemap::ctor: Failed to load texture " << filePaths[i] << " - " << stbi_failure_reason() << std::endl;
			std::cout << "Loading default texture." << std::endl << std::endl;
			
			data = stbi_load(CUBEMAP_PATHS[i].c_str(), &width, &height, &num_channels, 4);
			if (!data) std::cout << "ERROR::Cubemap::ctor: Failed to load default texture" << CUBEMAP_PATHS[i] << " - " << stbi_failure_reason() << std::endl << std::endl;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap::Cubemap(Cubemap&& other) noexcept
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

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept
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

Cubemap::~Cubemap() { cleanup(); }

void Cubemap::cleanup() { glDeleteTextures(1, &_id); }

GLuint Cubemap::get_ID() const { return _id; }