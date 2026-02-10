#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

const enum TextureSlots {
	TEX_SLOT_TARGET,		// For rendering target or temp work (like tex creation)
	TEX_SLOT_DIFFUSE,		// Diffuse or Skybox texture
	TEX_SLOT_AMBIENT,		// Ambient Cubemap (mostly for Skybox reflection)
};

class Shader
{
private:

	GLuint _id;
	std::unordered_map<std::string, GLint> _uniform_locations_cache;

	GLint get_uniform_location(const GLchar* name);

public:

	Shader(const std::string vertex_source_path, const std::string fragment_source_path);

	// Copy constructor/assignment are forbidden: we don't want multiple instances pointing to the same OpenGL data, since it can be deleted by them
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	~Shader();

	GLuint get_ID() const;
	void set_uniform_1b(const std::string &name, const bool value);
	void set_uniform_1i(const std::string &name, const int value);
	void set_uniform_1f(const std::string &name, const float value);
	void set_uniform_uvec3(const std::string &name, const glm::uvec3& vector);
	void set_uniform_vec3(const std::string &name, const glm::vec3& vector);
	void set_uniform_mat3(const std::string &name, const glm::mat3& matrix);
	void set_uniform_mat4(const std::string &name, const glm::mat4& matrix);
};