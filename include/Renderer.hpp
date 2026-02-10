#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Shader.hpp>
#include <Camera.hpp>
#include <Model.hpp>
#include <utils/Vertex.hpp>
#include <utils/Mesh.hpp>
#include <utils/Cubemap.hpp>

enum SKYBOX
{
	MOONRISE	= 0,
	DUSK		= 1,
	HILLS		= 2,
	ALIEN_CAVE	= 3,
	FANTASY_SKY	= 4,
};

enum PARTICLES_RENDERING
{
	POINTS	= 0,
	QUADS	= 1,
	FLUID	= 2,
};

enum PARTICLES_COLOR
{
	VELOCITY 			= 0,
	VELOCITY_COMPONENTS	= 1,
	DIFFUSE				= 2
};

enum FLUID_PASS
{
	THICKNESS	= 0,
	DEPTH 		= 1,
	NORMALS		= 2,
	WHITEWATER	= 3,
	FINAL		= 4
};

class Renderer
{
private:

	static const std::string LIT_TEX_VERT_SHADER_PATH, LIT_TEX_FRAG_SHADER_PATH;
	static const std::string SKYBOX_VERT_SHADER_PATH, SKYBOX_FRAG_SHADER_PATH;
	static const std::string PARTICLE_POINT_VERT_SHADER_PATH, PARTICLE_POINT_FRAG_SHADER_PATH;
	static const std::string PARTICLE_QUAD_VERT_SHADER_PATH, PARTICLE_QUAD_FRAG_SHADER_PATH;
	static const std::string PARTICLE_THICKNESS_FRAG_SHADER_PATH;
	static const std::string PARTICLE_DEPTH_FRAG_SHADER_PATH;
	static const std::string CELL_VERT_SHADER_PATH, CELL_FRAG_SHADER_PATH;
	static const std::string SCREEN_QUAD_VERT_SHADER_PATH, SCREEN_QUAD_FRAG_SHADER_PATH;
	static const std::string BLUR_FRAG_SHADER_PATH;
	static const std::string FLUID_NORMALS_FRAG_SHADER_PATH;
	static const std::string FLUID_COMP_FRAG_SHADER_PATH;
	static const std::string WHITEWATER_VERT_SHADER_PATH, WHITEWATER_FRAG_SHADER_PATH;
	static const glm::vec4 SCREEN_QUAD_VBO[6];
	static const std::vector<Vertex> SKYBOX_VBO;
	static const std::vector<GLuint> SKYBOX_EBO;
	static const std::string DEFAULT_TEXTURE_PATH;
	static const unsigned int MAX_BLUR_RADIUS = 10;	// Same as hardcoded max radius in shader

	Shader _lit_textured_shader;																				// Opaque model shader
	Shader _skybox_shader;
	Shader _particle_point_shader, _particle_quad_shader, _particle_thickness_shader, _particle_depth_shader;	// Impostor particle shaders
	Shader _grid_cell_shader;
	Shader _screen_quad_shader, _blur_shader, _fluid_normals_shader, _fluid_comp_shader, _whitewater_shader;	// Post-process tex shaders
	GLuint _offscreen_FBO, _reflections_cubemap_FBO;															// FrameBuffers
	GLuint _offscreen_depth_RBO, _reflections_cubemap_depth_RBO, _fluid_depth_RBO;								// RenderBuffers
	GLuint _scene_color_buffer;
	GLuint _fluid_color_buffer, _fluid_color_buffer_temp;
	GLuint _fluid_depth_map, _fluid_depth_map_temp;
	GLuint _fluid_thickness_map, _fluid_thickness_map_temp;
	GLuint _reflections_cubemap; 
	GLuint _screen_quad_VAO;
	Mesh _skybox_cube;
	Cubemap _skyboxes[5];
	Texture _default_texture;

	int _window_width, _window_height;
	int _offscreen_rendering_width, _offscreen_rendering_height;
	Camera _cubemap_camera;
	int _cubemap_size;
	float _gaussian_weights[MAX_BLUR_RADIUS + 1];	// Same as hardcoded max radius in shader + position 0

public:

	SKYBOX skybox_idx = MOONRISE;
	PARTICLES_RENDERING particles_rendering_mode = FLUID;
	float particles_radius = 0.6f;
	PARTICLES_COLOR particles_color = VELOCITY;
	FLUID_PASS fluid_pass = FINAL;
	bool fluid_show_whitewater = true;
	float particle_thickness = 0.020f;
	glm::vec3 fluid_light_absorbance = glm::vec3(0.92f, 0.30f, 0.10f);
	float fluid_refraction_strength = 0.020f;
	float fluid_fresnel_pow = 3.00f;
	int fluid_depth_blur_passes = 4;
	float fluid_blur_radius = 0.35f;
	float fluid_blur_smoothness = 4.0f;
	float fluid_blur_depth = 2.0f;

	GLfloat clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	Renderer(int window_width, int window_height);

	void on_framebuffer_size_change(int new_width, int new_height);

	void clear() const;

	void draw_lit_textured(const Camera& camera, const glm::vec3 light_direction, const std::vector<Model*>* models, bool textured = true);
	
	void draw_skybox(const Camera& camera, const Cubemap* skybox = nullptr);

	void draw_MPM_particles(
		const Camera& camera,
		const GLuint particles_VAO, 
		const GLuint particles_num, 
		const glm::vec3 position,
		const glm::vec3 light_direction);

	void draw_fluid(
		const Camera& camera,
		const GLuint particles_VAO, 
		const GLuint particles_num, 
		const GLuint whitewater_VAO, 
		const GLuint whitewater_start_idx,
		const GLuint whitewater_num,
		const glm::vec3 sim_position, 
		const glm::vec3 sim_center,
		const float water_level,
		const std::vector<Model*>* reflected_models, 
		const bool reflected_models_textured,
		const glm::vec3 light_direction);

	void draw_MPM_grid(
		const Camera& camera, 
		const glm::vec3 position, 
		const GLuint grid_VAO, 
		const GLuint cells_num, 
		const glm::uvec3& grid_size);

private:

	void _render_reflections_map(const glm::vec3 light_direction, const std::vector<Model*>* reflected_models, bool textured = true);

	void _render_whitewater(
		const Camera& camera, 
		const GLuint whitewater_VAO, 
		const GLuint whitewater_start_idx, 
		const GLuint whitewater_num, 
		const glm::vec3 sim_position);

	void _fluid_thickness_pass(
		const Camera& camera,
		const GLuint particles_VAO, 
		const GLuint particles_num,
		const glm::vec3 sim_position);

	void _fluid_depth_pass(
		const Camera& camera,
		const GLuint particles_VAO, 
		const GLuint particles_num,
		const glm::vec3 sim_position);

	void _fluid_normals_pass(
		const Camera& camera);

	void _fluid_composition_pass(
		const Camera& camera,
		const bool dynamic_reflections);

	void _blur(
		const Camera& camera,
		const GLsizei width,
		const GLsizei height,
		const GLuint texture,
		const GLuint texture_temp,
		const GLuint depth_map_hor_pass,
		const GLuint depth_map_vert_pass,
		const unsigned int passes);

	glm::vec2 _get_texel_size(GLuint texture);

	float* _compute_gaussian_weights();
};