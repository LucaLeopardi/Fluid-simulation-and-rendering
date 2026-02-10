#include <Renderer.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

const std::string Renderer::LIT_TEX_VERT_SHADER_PATH = "assets/shaders/lit_textured.vert";
const std::string Renderer::LIT_TEX_FRAG_SHADER_PATH = "assets/shaders/lit_textured.frag";
const std::string Renderer::SKYBOX_VERT_SHADER_PATH = "assets/shaders/skybox.vert";
const std::string Renderer::SKYBOX_FRAG_SHADER_PATH = "assets/shaders/skybox.frag";
const std::string Renderer::PARTICLE_POINT_VERT_SHADER_PATH = "assets/shaders/particle_point.vert";
const std::string Renderer::PARTICLE_POINT_FRAG_SHADER_PATH = "assets/shaders/particle_point.frag";
const std::string Renderer::PARTICLE_QUAD_VERT_SHADER_PATH = "assets/shaders/particle_quad.vert";
const std::string Renderer::PARTICLE_QUAD_FRAG_SHADER_PATH = "assets/shaders/particle_quad.frag";
const std::string Renderer::PARTICLE_THICKNESS_FRAG_SHADER_PATH = "assets/shaders/particle_thickness.frag";
const std::string Renderer::PARTICLE_DEPTH_FRAG_SHADER_PATH = "assets/shaders/particle_depth.frag";
const std::string Renderer::CELL_VERT_SHADER_PATH = "assets/shaders/cell.vert";
const std::string Renderer::CELL_FRAG_SHADER_PATH = "assets/shaders/cell.frag";
const std::string Renderer::SCREEN_QUAD_VERT_SHADER_PATH = "assets/shaders/screen_quad.vert";
const std::string Renderer::SCREEN_QUAD_FRAG_SHADER_PATH = "assets/shaders/screen_quad.frag";
const std::string Renderer::BLUR_FRAG_SHADER_PATH = "assets/shaders/blur.frag";
const std::string Renderer::FLUID_NORMALS_FRAG_SHADER_PATH = "assets/shaders/fluid_normals.frag";
const std::string Renderer::FLUID_COMP_FRAG_SHADER_PATH = "assets/shaders/fluid_comp.frag";
const std::string Renderer::WHITEWATER_VERT_SHADER_PATH = "assets/shaders/whitewater_quad.vert";
const std::string Renderer::WHITEWATER_FRAG_SHADER_PATH = "assets/shaders/whitewater_quad.frag";
const glm::vec4 Renderer::SCREEN_QUAD_VBO[6] = {
	// Position			// Tex coords
	{-1.0f, -1.0f,		0.0f, 0.0f},
	{ 1.0f, -1.0f,		1.0f, 0.0f},
	{-1.0f,  1.0f,		0.0f, 1.0f},

	{-1.0f,  1.0f,		0.0f, 1.0f},
	{ 1.0f, -1.0f,		1.0f, 0.0f},
	{ 1.0f,  1.0f,		1.0f, 1.0f},
};
const std::vector<Vertex> Renderer::SKYBOX_VBO = {
	// Position				// Normal				// Tex Coords	// Base color
	// Front
	{{-1.0f, -1.0f,  1.0f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 0
	{{ 1.0f, -1.0f,  1.0f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 1
	{{-1.0f,  1.0f,  1.0f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 2
	{{ 1.0f,  1.0f,  1.0f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 3
	// Right
	{{ 1.0f, -1.0f,  1.0f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 4
	{{ 1.0f, -1.0f, -1.0f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 5
	{{ 1.0f,  1.0f,  1.0f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 6
	{{ 1.0f,  1.0f, -1.0f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 7
	// Back
	{{ 1.0f, -1.0f, -1.0f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 8
	{{-1.0f, -1.0f, -1.0f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 9
	{{ 1.0f,  1.0f, -1.0f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 10
	{{-1.0f,  1.0f, -1.0f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 11
	// Left
	{{-1.0f, -1.0f, -1.0f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 12
	{{-1.0f, -1.0f,  1.0f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 13
	{{-1.0f,  1.0f, -1.0f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 14
	{{-1.0f,  1.0f,  1.0f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 15
	// Top
	{{-1.0f,  1.0f,  1.0f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 16
	{{ 1.0f,  1.0f,  1.0f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 17
	{{-1.0f,  1.0f, -1.0f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 18
	{{ 1.0f,  1.0f, -1.0f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 19
	// Bottom
	{{-1.0f, -1.0f, -1.0f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 20
	{{ 1.0f, -1.0f, -1.0f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 21
	{{-1.0f, -1.0f,  1.0f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 22
	{{ 1.0f, -1.0f,  1.0f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{1.0f, 1.0f, 1.0f, 1.0f}},	// 23
};
const std::vector<GLuint> Renderer::SKYBOX_EBO = {
	// Front
	2, 1, 0,
	1, 2, 3,
	// Right
	6, 5, 4,
	5, 6, 7,
	// Back
	10, 9, 8,
	9, 10, 11,
	// Left
	14, 13, 12,
	13, 14, 15,
	// Top
	18, 17, 16,
	17, 18, 19,
	// Bottom
	22, 21, 20,
	21, 22, 23
};
const std::string Renderer::DEFAULT_TEXTURE_PATH = "assets/textures/default_texture.png";


// Shaders and textures are initialized in the constructor
Renderer::Renderer(int window_width, int window_height) :
	_window_width(window_width),
	_window_height(window_height),
	_offscreen_rendering_width(window_width / 2),
	_offscreen_rendering_height(window_height / 2),
	_cubemap_size( window_height / 2),
	_lit_textured_shader(LIT_TEX_VERT_SHADER_PATH, LIT_TEX_FRAG_SHADER_PATH),
	_skybox_shader(SKYBOX_VERT_SHADER_PATH, SKYBOX_FRAG_SHADER_PATH),
	_particle_point_shader(PARTICLE_POINT_VERT_SHADER_PATH, PARTICLE_POINT_FRAG_SHADER_PATH),
	_particle_quad_shader(PARTICLE_QUAD_VERT_SHADER_PATH, PARTICLE_QUAD_FRAG_SHADER_PATH),
	_particle_thickness_shader(PARTICLE_QUAD_VERT_SHADER_PATH, PARTICLE_THICKNESS_FRAG_SHADER_PATH),
	_particle_depth_shader(PARTICLE_QUAD_VERT_SHADER_PATH, PARTICLE_DEPTH_FRAG_SHADER_PATH),
	_grid_cell_shader(CELL_VERT_SHADER_PATH, CELL_FRAG_SHADER_PATH),
	_screen_quad_shader(SCREEN_QUAD_VERT_SHADER_PATH, SCREEN_QUAD_FRAG_SHADER_PATH),
	_blur_shader(SCREEN_QUAD_VERT_SHADER_PATH, BLUR_FRAG_SHADER_PATH),
	_fluid_normals_shader(SCREEN_QUAD_VERT_SHADER_PATH, FLUID_NORMALS_FRAG_SHADER_PATH),
	_fluid_comp_shader(SCREEN_QUAD_VERT_SHADER_PATH, FLUID_COMP_FRAG_SHADER_PATH),
	_whitewater_shader(WHITEWATER_VERT_SHADER_PATH, WHITEWATER_FRAG_SHADER_PATH),
	_skybox_cube(SKYBOX_VBO, SKYBOX_EBO),
	_skyboxes {
		Cubemap({																					// 0 - Qwantani Moonrise by Greg Zaal and Jarod Guest @ Polyhaven - https://polyhaven.com/a/qwantani_moonrise_puresky
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/PX.png",	// PX
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/NX.png",	// NX
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/PY.png",	// PY
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/NY.png",	// NY
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/PZ.png",	// PZ
			"assets/textures/Qwantani Moonrise (by Greg Zaal and Jarod Guest - Polyhaven)/NZ.png"	// NZ
		}),
		Cubemap({																				// 1 - Qwuantani Dusk by Greg Zaal and Jarod Guest @ Polyhaven - https://polyhaven.com/a/qwantani_dusk_2
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/PX.png",	// PX
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/NX.png",	// NX
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/PY.png",	// PY
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/NY.png",	// NY
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/PZ.png",	// PZ
			"assets/textures/Qwuantani dusk (by Greg Zaal and Jarod Guest - Polyhaven)/NZ.png"	// NZ
		}),
		Cubemap({																							// 2 - Golden Gate Hills by Dimitrios Savva and Jarod Guest @ Polyhaven - https://polyhaven.com/a/golden_gate_hills
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/PX.png",	// PX
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/NX.png",	// NX
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/PY.png",	// PY
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/NY.png",	// NY
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/PZ.png",	// PZ
			"assets/textures/Golden Gate Hills (by Dimitrios Savva and Jarod Guest - Polyhaven)/NZ.png"		// NZ
		}),
		Cubemap({															// 3 - Alien Cave by Jungle Jim @ Sketchfab - https://skfb.ly/pzUGK
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/PX.png",	// PX
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/NX.png",	// NX
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/PY.png",	// PY
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/NY.png",	// NY
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/PZ.png",	// PZ
			"assets/textures/Alien cave (by Junge Jim - Sketchfab)/NZ.png"	// NZ
		}),
		Cubemap({															// 4 - Fantasy Sky by Hannes @ Sketchfab - https://skfb.ly/6u9DR
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/PX.png",	// PX
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/NX.png",	// NX
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/PY.png",	// PY
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/NY.png",	// NY
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/PZ.png",	// PZ
			"assets/textures/Fantasy sky (by Hannes - Sketchfab)/NZ.png"	// NZ
		})
	},
	_default_texture(DEFAULT_TEXTURE_PATH),
	_cubemap_camera(_cubemap_size, _cubemap_size, 1.571f, 0.1f, 500.0f)
{

	// Backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	// Depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);	// To not discard fragments with Z=1.0 in NDC (which is explicitly set with Skybox)
	// Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// GL_POINTS mode point size, set in vertex Shader
	glEnable(GL_PROGRAM_POINT_SIZE);


	// Offscreen rendering Framebuffer
	glGenFramebuffers(1, &_offscreen_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _offscreen_FBO);

	glGenTextures(1, &_scene_color_buffer);
	glBindTexture(GL_TEXTURE_2D, _scene_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_width, window_height, 0, GL_RGBA, GL_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _scene_color_buffer, 0);
	
	glGenRenderbuffers(1, &_fluid_depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, _fluid_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fluid_depth_RBO);
	
	glGenRenderbuffers(1, &_offscreen_depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, _offscreen_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _offscreen_rendering_width, _offscreen_rendering_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _offscreen_depth_RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::RENDERER::CTOR:: Could not complete offscreen rendering Framebuffer!" << std::endl;

	// Fluid rendering textures for post-process passes
	glGenTextures(1, &_fluid_color_buffer);
	glBindTexture(GL_TEXTURE_2D, _fluid_color_buffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, window_width, window_height, 0, GL_RGBA, GL_BYTE, NULL);	// Format to correctly store signed vec components clamped to [-1.0, 1.0]
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &_fluid_color_buffer_temp);
	glBindTexture(GL_TEXTURE_2D, _fluid_color_buffer_temp);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, window_width, window_height, 0, GL_RGBA, GL_BYTE, NULL);	// Format to correctly store signed vec components clamped to [-1.0, 1.0]
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &_fluid_thickness_map);
	glBindTexture(GL_TEXTURE_2D, _fluid_thickness_map);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _offscreen_rendering_width, _offscreen_rendering_height, 0, GL_RED, GL_FLOAT, NULL);	// Half res. for performance
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &_fluid_thickness_map_temp);
	glBindTexture(GL_TEXTURE_2D, _fluid_thickness_map_temp);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _offscreen_rendering_width, _offscreen_rendering_height, 0, GL_RED, GL_FLOAT, NULL);	// Half res. for performance
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &_fluid_depth_map);
	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &_fluid_depth_map_temp);
	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map_temp);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Fluid dynamic environment cubemap
	glGenTextures(1, &_reflections_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflections_cubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &_reflections_cubemap_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _reflections_cubemap_FBO);

	glGenRenderbuffers(1, &_reflections_cubemap_depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, _reflections_cubemap_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _cubemap_size, _cubemap_size);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _reflections_cubemap, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _reflections_cubemap_depth_RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::RENDERER::CTOR:: Could not complete environment map Framebuffer!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);


	// Screen texture shader quad
	glGenVertexArrays(1, &_screen_quad_VAO);
	glBindVertexArray(_screen_quad_VAO);
	
	GLuint screen_quad_VBO;
	glGenBuffers(1, &screen_quad_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, screen_quad_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_QUAD_VBO), &SCREEN_QUAD_VBO[0], GL_STATIC_DRAW);
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(glm::vec4), (void*) 0);
	// Tex coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(glm::vec4), (void*) (2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Name OpenGL Objects for debugging
	glObjectLabel(GL_PROGRAM, _lit_textured_shader.get_ID(), -1, "unlit_shader");
	glObjectLabel(GL_PROGRAM, _skybox_shader.get_ID(), -1, "skybox_shader");
	glObjectLabel(GL_PROGRAM, _particle_point_shader.get_ID(), -1, "particle_point_shader");
	glObjectLabel(GL_PROGRAM, _particle_quad_shader.get_ID(), -1, "particle_quad_shader");
	glObjectLabel(GL_PROGRAM, _particle_depth_shader.get_ID(), -1, "particle_depth_shader");
	glObjectLabel(GL_PROGRAM, _particle_thickness_shader.get_ID(), -1, "particle_thickness_shader");
	glObjectLabel(GL_PROGRAM, _grid_cell_shader.get_ID(), -1, "grid_cell_shader");
	glObjectLabel(GL_PROGRAM, _screen_quad_shader.get_ID(), -1, "screen_quad_shader");
	glObjectLabel(GL_PROGRAM, _blur_shader.get_ID(), -1, "blur_shader");
	glObjectLabel(GL_PROGRAM, _fluid_normals_shader.get_ID(), -1, "fluid_normals_shader");
	glObjectLabel(GL_PROGRAM, _fluid_comp_shader.get_ID(), -1, "fluid_comp_shader");
	glObjectLabel(GL_PROGRAM, _whitewater_shader.get_ID(), -1, "whitewater_shader");
	glObjectLabel(GL_TEXTURE, _scene_color_buffer, -1, "scene_color_buffer");
	glObjectLabel(GL_TEXTURE, _fluid_color_buffer, -1, "fluid_color_buffer");
	glObjectLabel(GL_TEXTURE, _fluid_color_buffer_temp, -1, "fluid_color_buffer_temp");
	glObjectLabel(GL_TEXTURE, _fluid_depth_map, -1, "fluid_depth_map");
	glObjectLabel(GL_TEXTURE, _fluid_depth_map_temp, -1, "fluid_depth_map_temp");
	glObjectLabel(GL_TEXTURE, _fluid_thickness_map, -1, "fluid_thickness_map");
	glObjectLabel(GL_TEXTURE, _fluid_thickness_map_temp, -1, "fluid_thickness_map_temp");
	glObjectLabel(GL_TEXTURE, _skyboxes[0].get_ID(), -1, "default_skybox");
	glObjectLabel(GL_TEXTURE, _skyboxes[1].get_ID(), -1, "fantasy_sky_skybox");
	glObjectLabel(GL_TEXTURE, _skyboxes[2].get_ID(), -1, "alien_cave_skybox");
	glObjectLabel(GL_TEXTURE, _skyboxes[3].get_ID(), -1, "malibu_skybox");
	glObjectLabel(GL_TEXTURE, _reflections_cubemap, -1, "reflections_cubemap");
	glObjectLabel(GL_RENDERBUFFER, _fluid_depth_RBO, -1, "offscreen_renderBuffer_depth");
	glObjectLabel(GL_RENDERBUFFER, _reflections_cubemap_depth_RBO, -1, "environment_map_renderBuffer_depth");
	glObjectLabel(GL_FRAMEBUFFER, _offscreen_FBO, -1, "offscreen_frameBuffer");
	glObjectLabel(GL_FRAMEBUFFER, _reflections_cubemap_FBO, -1, "environment_map_frameBuffer");
}


void Renderer::on_framebuffer_size_change(int new_width, int new_height)
{
	_window_width = new_width;
	_window_height = new_height;
	_offscreen_rendering_width = new_width / 2;
	_offscreen_rendering_height = new_height / 2;
	_cubemap_size = new_height / 2;

	// Resize offscreen rendering target textures
	glBindTexture(GL_TEXTURE_2D, _scene_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, new_width, new_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, new_width, new_height, 0, GL_RGBA, GL_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_color_buffer_temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, new_width, new_height, 0, GL_RGBA, GL_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, new_width, new_height, 0, GL_RED, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map_temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, new_width, new_height, 0, GL_RED, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_thickness_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _offscreen_rendering_width, _offscreen_rendering_height, 0, GL_RED, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, _fluid_thickness_map_temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _offscreen_rendering_width, _offscreen_rendering_height, 0, GL_RED, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflections_cubemap);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, _cubemap_size, _cubemap_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	glBindRenderbuffer(GL_RENDERBUFFER, _fluid_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, new_width, new_height);
	
	glBindRenderbuffer(GL_RENDERBUFFER, _offscreen_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _offscreen_rendering_width, _offscreen_rendering_height);

	glBindRenderbuffer(GL_RENDERBUFFER, _reflections_cubemap_depth_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _cubemap_size, _cubemap_size);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


void Renderer::clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Renderer::draw_lit_textured(const Camera& camera, const glm::vec3 light_direction, const std::vector<Model*>* models, bool textured)
{
	if (!models) return;
	
	glUseProgram(_lit_textured_shader.get_ID());
	// Common Uniform data
	_lit_textured_shader.set_uniform_vec3("u_light_direction", glm::normalize(light_direction));
	_lit_textured_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_lit_textured_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_lit_textured_shader.set_uniform_1i("u_diffuse_texture", TEX_SLOT_DIFFUSE);
	_lit_textured_shader.set_uniform_1b("u_use_texture", textured);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	// Model-specific bindings
	for (const Model* model : *models) {
		_lit_textured_shader.set_uniform_mat4("u_model_mat", model->get_model_mat());
		_lit_textured_shader.set_uniform_mat3("u_normal_mat", model->get_normal_mat());
		for (const Mesh& mesh : model->meshes) {
			if (mesh.diffuse_tex && mesh.diffuse_tex->get_ID() != 0)
				glBindTexture(GL_TEXTURE_2D, mesh.diffuse_tex->get_ID());
			else
				glBindTexture(GL_TEXTURE_2D, _default_texture.get_ID());
			glBindVertexArray(mesh.get_VAO());
			glDrawElements(GL_TRIANGLES, mesh.get_EBO_length(), GL_UNSIGNED_INT, 0);
		}
	}
	// Leave no resources bound
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}


void Renderer::draw_skybox(const Camera& camera, const Cubemap* skybox)
{
	if (!skybox) skybox = &_skyboxes[skybox_idx];

	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(_skybox_shader.get_ID());
	
	_skybox_shader.set_uniform_mat4("u_view_mat", camera.get_view_rotation_mat());
	_skybox_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_skybox_shader.set_uniform_1i("u_cubemap", TEX_SLOT_SKYBOX);

	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_SKYBOX);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->get_ID());
	glBindVertexArray(_skybox_cube.get_VAO());
	glDrawElements(GL_TRIANGLES, _skybox_cube.get_EBO_length(), GL_UNSIGNED_INT, 0);

	// Leave no resources bound and re-enable depth write
	glDepthMask(GL_TRUE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}


void Renderer::draw_MPM_particles(
	const Camera& camera,
	const GLuint particles_VAO, 
	const GLuint particles_num, 
	const glm::vec3 position,
	const glm::vec3 light_direction)
{
	switch (particles_rendering_mode) {
		case PARTICLES_RENDERING::POINTS:
			glUseProgram(_particle_point_shader.get_ID());
			_particle_point_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), position));	// Simulation can only be translated, NOT rotated/scaled
			_particle_point_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
			_particle_point_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
			_particle_point_shader.set_uniform_1f("u_particle_radius", particles_radius);
			_particle_point_shader.set_uniform_1i("u_particle_color", particles_color);
			glBindVertexArray(particles_VAO);
			glDrawArraysInstanced(GL_POINTS, 0, 1, particles_num);
			break;
		case PARTICLES_RENDERING::QUADS:
			glUseProgram(_particle_quad_shader.get_ID());
			_particle_quad_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), position));	// Simulation can only be translated, NOT rotated/scaled
			_particle_quad_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
			_particle_quad_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
			_particle_quad_shader.set_uniform_1i("u_particle_color", particles_color);
			_particle_quad_shader.set_uniform_1f("u_particle_radius", particles_radius);
			_particle_quad_shader.set_uniform_vec3("u_light_direction", glm::normalize(light_direction));
			glBindVertexArray(particles_VAO);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles_num);
			break;
		default:
			break;
	}
}


bool update_reflections = true;	// Bool used to render reflections every 2 frames

void Renderer::draw_fluid(
	const Camera & camera,
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
	const glm::vec3 light_direction) 
{
	if (particles_num <= 0) return;

	// Before the fluid, render the dynamic reflections map from its POV
	if (update_reflections && reflected_models) {
		_cubemap_camera.set_position(glm::vec3(sim_center.x, water_level / 2.0f, sim_center.z));
		_render_reflections_map(light_direction, reflected_models, reflected_models_textured);
	}
	update_reflections = !update_reflections;

	// Draw whitewater particles BEFORE fluid so they can refracted within it.
	if (fluid_show_whitewater) {
		_render_whitewater(
			camera,
			whitewater_VAO,
			whitewater_start_idx,
			whitewater_num,
			sim_position
		);
	}

	// Blit scene color and depth into offscreen FBO. Color is needed for refraction, depth for occlusion.
	glBindFramebuffer(GL_FRAMEBUFFER, _offscreen_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _scene_color_buffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fluid_depth_RBO);
	glBlitNamedFramebuffer(0, _offscreen_FBO, 0, 0, _window_width, _window_height, 0, 0, _window_width, _window_height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _offscreen_depth_RBO);
	glBlitNamedFramebuffer(0, _offscreen_FBO, 0, 0, _window_width, _window_height, 0, 0, _offscreen_rendering_width, _offscreen_rendering_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// Render a fluid thickness map, used for refraction and light absorbance.
	_fluid_thickness_pass(
		camera,
		particles_VAO,
		particles_num,
		sim_position
	);

	// Render a linear depth map (aka distance from camera), used to calculate surface normals and depth-weighted blur. 
	_fluid_depth_pass(
		camera,
		particles_VAO,
		particles_num,
		sim_position
	);

	_compute_gaussian_weights();
	
	// Blur the depth map
	_blur(
		camera,
		_window_width,
		_window_height,
		_fluid_depth_map,
		_fluid_depth_map_temp,
		_fluid_depth_map,
		_fluid_depth_map_temp,
		fluid_depth_blur_passes
	);
	
	// Blur the thickness map.
	_blur(
		camera,
		_offscreen_rendering_width,
		_offscreen_rendering_height,
		_fluid_thickness_map,
		_fluid_thickness_map_temp,
		_fluid_depth_map,
		_fluid_depth_map,
		2
	);

	// Compute surface view-space normals from linear depth.
	_fluid_normals_pass(
		camera
	);

	// Final render of the fluid, using view-space normals to compute reflection and refraction.
	_fluid_composition_pass(
		camera,
		reflected_models ? true : false
	);

	// Final pass: copy to screen
	GLuint final_tex;
	switch (fluid_pass) {
		case THICKNESS : final_tex = _fluid_thickness_map; break;
		case DEPTH : final_tex = _fluid_depth_map; break;
		case NORMALS : final_tex = _fluid_color_buffer_temp; break;
		case WHITEWATER : final_tex = _scene_color_buffer; break;
		case FINAL:
		default: final_tex = _fluid_color_buffer;
	}
	// Reset depth test and write, blending function
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	// Write to screen
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Shader setup
	glUseProgram(_screen_quad_shader.get_ID());
	_screen_quad_shader.set_uniform_1i("u_texture", TEX_SLOT_TEMP);
	// Draw
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_TEMP);
	glBindTexture(GL_TEXTURE_2D, final_tex);	
	glBindVertexArray(_screen_quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	// Unbind resources
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_TEMP);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DEPTH);
	glBindTexture(GL_TEXTURE_2D, 0);	
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_THICKNESS);
	glBindTexture(GL_TEXTURE_2D, 0);	
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_AMBIENT);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}


void Renderer::draw_MPM_grid(
	const Camera& camera, 
	const glm::vec3 position, 
	const GLuint grid_VAO, 
	const GLuint cells_num, 
	const glm::uvec3& grid_size)
{
	glUseProgram(_grid_cell_shader.get_ID());
	_grid_cell_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), position));	// Simulation can only be translated, NOT rotated/scaled
	_grid_cell_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_grid_cell_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_grid_cell_shader.set_uniform_uvec3("u_grid_size", grid_size);
	glDepthMask(GL_FALSE);
	glBindVertexArray(grid_VAO);
	glDrawArrays(GL_POINTS, 0, cells_num);

	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
}


void Renderer::_render_reflections_map(const glm::vec3 light_direction, const std::vector<Model*>* reflected_models, bool textured)
{
	if (!reflected_models) return;

	glBindFramebuffer(GL_FRAMEBUFFER, _reflections_cubemap_FBO);
	glViewport(0, 0, _cubemap_size, _cubemap_size);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	// Up directions are flipped because cubemaps follow RenderMan conventions, so faces are rendered "upside-down"
	// Positive X
	_cubemap_camera.set_direction(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);
	// Negative X
	_cubemap_camera.set_direction(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);
	// Positive Y
	_cubemap_camera.set_direction(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);
	// Negative Y
	_cubemap_camera.set_direction(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);
	// Positive Z
	_cubemap_camera.set_direction(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);
	// Negative Z
	_cubemap_camera.set_direction(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, _reflections_cubemap, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_lit_textured(_cubemap_camera, light_direction, reflected_models, textured);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _window_width, _window_height);
}

void Renderer::_render_whitewater(
	const Camera& camera, 
	const GLuint whitewater_VAO, 
	const GLuint whitewater_start_idx, 
	const GLuint whitewater_num, 
	const glm::vec3 sim_position)
{
	glUseProgram(_whitewater_shader.get_ID());
	_whitewater_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), sim_position));	// Simulation can only be translated, NOT rotated/scaled
	_whitewater_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_whitewater_shader.set_uniform_mat4("u_inv_view_mat", camera.get_inverse_view_mat());
	_whitewater_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_whitewater_shader.set_uniform_1f("u_particle_radius", particles_radius / 2.0f);
	_whitewater_shader.set_uniform_1i("u_ambient", TEX_SLOT_AMBIENT);
	// Draw
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_AMBIENT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxes[skybox_idx].get_ID());
	glBindVertexArray(whitewater_VAO);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, whitewater_num, whitewater_start_idx);
}


void Renderer::_fluid_thickness_pass(
	const Camera& camera, 
	const GLuint particles_VAO, 
	const GLuint particles_num, 
	const glm::vec3 sim_position)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Discarded fragments in post-process shaders will keep alpha <= 0.001 to signal a null value
	// Pass setup
	glBindFramebuffer(GL_FRAMEBUFFER, _offscreen_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fluid_thickness_map, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _offscreen_depth_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, _offscreen_rendering_width, _offscreen_rendering_height);
	
	glEnable(GL_DEPTH_TEST);		// Enable depth test 
	glDepthMask(GL_FALSE);			// BUT disable depth write
	glBlendFunc(GL_ONE, GL_ONE);	// Additive rendering to sum particles thickness
	
	glUseProgram(_particle_thickness_shader.get_ID());
	_particle_thickness_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), sim_position));	// Simulation can only be translated, NOT rotated/scaled
	_particle_thickness_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_particle_thickness_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_particle_thickness_shader.set_uniform_1f("u_particle_radius", particles_radius);
	_particle_thickness_shader.set_uniform_1f("u_particle_thickness", particle_thickness);
	
	glBindVertexArray(particles_VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles_num);

	// Restore standard render configuration
	glViewport(0, 0, _window_width, _window_height);
	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Renderer::_fluid_depth_pass(
	const Camera& camera, 
	const GLuint particles_VAO, 
	const GLuint particles_num, 
	const glm::vec3 sim_position)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Discarded fragments in post-process shaders will keep alpha <= 0.001 to signal a null value
	// Pass setup
	glBindFramebuffer(GL_FRAMEBUFFER, _offscreen_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fluid_depth_map, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fluid_depth_RBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, _window_width, _window_height);
	
	glUseProgram(_particle_depth_shader.get_ID());
	_particle_depth_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), sim_position));	// Simulation can only be translated, NOT rotated/scaled
	_particle_depth_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_particle_depth_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_particle_depth_shader.set_uniform_1f("u_particle_radius", particles_radius);

	glBindVertexArray(particles_VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles_num);

	// Restore standard render configuration
	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Renderer::_fluid_normals_pass(
	const Camera & camera)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Discarded fragments in post-process shaders will keep alpha <= 0.001 to signal a null value
	// Pass setup
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fluid_color_buffer_temp, 0);	
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(_fluid_normals_shader.get_ID());
	_fluid_normals_shader.set_uniform_1i("u_depth_map", TEX_SLOT_DEPTH);
	_fluid_normals_shader.set_uniform_vec2("u_texel_size", _get_texel_size(_fluid_depth_map));
	_fluid_normals_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	//Draw
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DEPTH);
	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map);
	glBindVertexArray(_screen_quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::_fluid_composition_pass(const Camera & camera, const bool dynamic_reflections)
{
	// Pass setup
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fluid_color_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_fluid_comp_shader.get_ID());
	_fluid_comp_shader.set_uniform_1i("u_depth_map", TEX_SLOT_DEPTH);
	_fluid_comp_shader.set_uniform_1i("u_thickness_map", TEX_SLOT_THICKNESS);
	_fluid_comp_shader.set_uniform_1i("u_normals_map", TEX_SLOT_TEMP);
	_fluid_comp_shader.set_uniform_1i("u_scene_texture", TEX_SLOT_DIFFUSE);
	_fluid_comp_shader.set_uniform_1i("u_skybox", TEX_SLOT_SKYBOX);
	_fluid_comp_shader.set_uniform_1b("u_dynamic_reflections", dynamic_reflections);
	_fluid_comp_shader.set_uniform_1i("u_reflections_cubemap", TEX_SLOT_REFLECTIONS);
	_fluid_comp_shader.set_uniform_vec3("u_reflections_cubemap_position", _cubemap_camera.get_position());
	_fluid_comp_shader.set_uniform_1f("u_reflections_cubemap_size", _cubemap_camera.get_z_far());
	_fluid_comp_shader.set_uniform_1f("u_refraction_strength", fluid_refraction_strength);
	_fluid_comp_shader.set_uniform_1f("u_fresnel_pow", fluid_fresnel_pow);
	_fluid_comp_shader.set_uniform_vec3("u_light_absorbance", fluid_light_absorbance);
	_fluid_comp_shader.set_uniform_mat4("u_inv_view_mat", camera.get_inverse_view_mat());
	_fluid_comp_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());

	glBindVertexArray(_screen_quad_VAO);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DEPTH);
	glBindTexture(GL_TEXTURE_2D, _fluid_depth_map);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_THICKNESS);
	glBindTexture(GL_TEXTURE_2D, _fluid_thickness_map);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_TEMP);
	glBindTexture(GL_TEXTURE_2D, _fluid_color_buffer_temp);	// Normals
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	glBindTexture(GL_TEXTURE_2D, _scene_color_buffer);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_SKYBOX);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxes[skybox_idx].get_ID());	
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_REFLECTIONS);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflections_cubemap);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Renderer::_blur(
	const Camera& camera,
	const GLsizei width,
	const GLsizei height,
	const GLuint texture,
	const GLuint texture_temp,
	const GLuint depth_map_hor_pass,
	const GLuint depth_map_vert_pass,
	const unsigned int passes)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Discarded fragments in post-process shaders will keep alpha <= 0.001 to signal a null value
	// Pass setup
	glBindFramebuffer(GL_FRAMEBUFFER, _offscreen_FBO);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glUseProgram(_blur_shader.get_ID());
	_blur_shader.set_uniform_1i("u_texture", TEX_SLOT_TEMP);
	_blur_shader.set_uniform_1i("u_depth_map", TEX_SLOT_DEPTH);
	_blur_shader.set_uniform_vec2("u_texel_size", _get_texel_size(texture));
	_blur_shader.set_uniform_1f("u_blur_radius_world_size", fluid_blur_radius);
	_blur_shader.set_uniform_1fv("u_blur_weights", MAX_BLUR_RADIUS + 1, _gaussian_weights);
	_blur_shader.set_uniform_1f("u_blur_depth", fluid_blur_depth);
	_blur_shader.set_uniform_1i("u_camera_width", camera.get_width());
	_blur_shader.set_uniform_1f("u_camera_hor_scale", camera.get_horizontal_scale());

	glBindVertexArray(_screen_quad_VAO);
	for (int i = 0; i < passes; ++i) { 
		// Horizontal pass
		_blur_shader.set_uniform_1b("u_is_vertical", false);
		// Setup
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_temp, 0);	
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + TEX_SLOT_TEMP);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DEPTH);
		glBindTexture(GL_TEXTURE_2D, depth_map_hor_pass);
		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Vertical pass
		_blur_shader.set_uniform_1b("u_is_vertical", true);
		// Pass setup
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);	
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + TEX_SLOT_TEMP);
		glBindTexture(GL_TEXTURE_2D, texture_temp);
		glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DEPTH);
		glBindTexture(GL_TEXTURE_2D, depth_map_vert_pass);
		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Restore standard render configuration
	glViewport(0, 0, _window_width, _window_height);
	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);
}


glm::vec2 Renderer::_get_texel_size(GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	int width, height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	glBindTexture(GL_TEXTURE_2D, 0);

	return glm::vec2(1.0f / width, 1.0f / height);
}


float * Renderer::_compute_gaussian_weights()
{
	for (int i = 0; i <= MAX_BLUR_RADIUS; ++i)
		_gaussian_weights[i] = exp((-i * i) / (2.0f * fluid_blur_smoothness * fluid_blur_smoothness));

	return _gaussian_weights;
}