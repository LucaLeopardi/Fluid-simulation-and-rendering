#pragma once

#include <vector>
#include <glad/glad.h>
#include <Shader.hpp>
#include <Camera.hpp>
#include <Model.hpp>
#include <utils/Vertex.hpp>
#include <utils/Mesh.hpp>
#include <utils/Cubemap.hpp>
#include <MPM/Particle.hpp>

class Renderer
{
private:

	static const std::string DEFAULT_MODEL_VERT_SHADER_PATH;
	static const std::string DEFAULT_MODEL_FRAG_SHADER_PATH;
	static const std::string DEFAULT_REFLECTIVE_VERT_SHADER_PATH;
	static const std::string DEFAULT_REFLECTIVE_FRAG_SHADER_PATH;
	static const std::string DEFAULT_SKYBOX_VERT_SHADER_PATH;
	static const std::string DEFAULT_SKYBOX_FRAG_SHADER_PATH;
	static const std::string DEFAULT_PARTICLE_VERT_SHADER_PATH;
	static const std::string DEFAULT_PARTICLE_FRAG_SHADER_PATH;
	static const std::string DEFAULT_CELL_VERT_SHADER_PATH;
	static const std::string DEFAULT_CELL_FRAG_SHADER_PATH;
	static const std::vector<Vertex> DEFAULT_SKYBOX_VBO;
	static const std::vector<GLuint> DEFAULT_SKYBOX_EBO;
	static const std::string DEFAULT_CUBEMAP_PATHS[6];
	static const std::string DEFAULT_TEXTURE_PATH;

	Shader _unlit_shader;
	Shader _reflective_shader;
	Shader _skybox_shader;
	Shader _particle_shader;
	Shader _grid_cell_shader;
	Mesh _skybox_cube;
	Cubemap _default_skybox;
	Texture _default_texture;

public:

	GLfloat clear_color[4] = {0.2f, 0.3f, 0.3f, 1.0f};

	Renderer();

	void clear() const;
	
	void draw_points(const Camera& camera, const std::vector<Mesh*>& meshes);

	// To draw Meshes directly (mostly for testing). Only uses unlit Shader.
	void draw_meshes_unlit(const Camera& camera, const std::vector<Mesh*>& meshes);

	void draw_unlit(const Camera& camera, const std::vector<Model*>& models);
	
	void draw_reflective(const Camera& camera, const std::vector<Model*>& models);

	void draw_reflective(const Camera& camera, const std::vector<Model*>& models, const Cubemap& skybox);

	void draw_skybox(const Camera& camera);

	void draw_skybox(const Camera& camera, const Cubemap& skybox);

	void draw_MPM_particles(const Camera& camera, const GLuint particles_VAO, const GLuint particles_num, const glm::vec3 position);

	void draw_MPM_grid(const Camera& camera, const glm::vec3 position, const GLuint grid_VAO, const GLuint cells_num, const glm::uvec3& grid_size);
};