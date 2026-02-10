#include <Renderer.hpp>
#include <glm/ext/matrix_transform.hpp>

const std::string Renderer::DEFAULT_MODEL_VERT_SHADER_PATH = "assets/shaders/unlit_textured.vert";
const std::string Renderer::DEFAULT_MODEL_FRAG_SHADER_PATH = "assets/shaders/unlit_textured.frag";
const std::string Renderer::DEFAULT_REFLECTIVE_VERT_SHADER_PATH = "assets/shaders/reflective.vert";
const std::string Renderer::DEFAULT_REFLECTIVE_FRAG_SHADER_PATH = "assets/shaders/reflective.frag";
const std::string Renderer::DEFAULT_SKYBOX_VERT_SHADER_PATH = "assets/shaders/skybox.vert";
const std::string Renderer::DEFAULT_SKYBOX_FRAG_SHADER_PATH = "assets/shaders/skybox.frag";
const std::string Renderer::DEFAULT_PARTICLE_VERT_SHADER_PATH = "assets/shaders/particle.vert";
const std::string Renderer::DEFAULT_PARTICLE_FRAG_SHADER_PATH = "assets/shaders/particle.frag";
const std::string Renderer::DEFAULT_CELL_VERT_SHADER_PATH = "assets/shaders/cell.vert";
const std::string Renderer::DEFAULT_CELL_FRAG_SHADER_PATH = "assets/shaders/cell.frag";
const std::vector<Vertex> Renderer::DEFAULT_SKYBOX_VBO = {
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
const std::vector<GLuint> Renderer::DEFAULT_SKYBOX_EBO = {
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
const std::string Renderer::DEFAULT_CUBEMAP_PATHS[6] = {
	"assets/textures/default_skybox/right.jpg",
	"assets/textures/default_skybox/left.jpg",
	"assets/textures/default_skybox/top.jpg",
	"assets/textures/default_skybox/bottom.jpg",
	"assets/textures/default_skybox/front.jpg",
	"assets/textures/default_skybox/back.jpg"
};
const std::string Renderer::DEFAULT_TEXTURE_PATH = "assets/textures/default_texture.png";

// Default shaders and textures are set in the constructor	// TODO: Refactor? Subroutines?
Renderer::Renderer() :
	_unlit_shader(DEFAULT_MODEL_VERT_SHADER_PATH, DEFAULT_MODEL_FRAG_SHADER_PATH),
	_reflective_shader(DEFAULT_REFLECTIVE_VERT_SHADER_PATH, DEFAULT_REFLECTIVE_FRAG_SHADER_PATH),
	_skybox_shader(DEFAULT_SKYBOX_VERT_SHADER_PATH, DEFAULT_SKYBOX_FRAG_SHADER_PATH),
	_particle_shader(DEFAULT_PARTICLE_VERT_SHADER_PATH, DEFAULT_PARTICLE_FRAG_SHADER_PATH),
	_grid_cell_shader(DEFAULT_CELL_VERT_SHADER_PATH, DEFAULT_CELL_FRAG_SHADER_PATH),
	_skybox_cube(DEFAULT_SKYBOX_VBO, DEFAULT_SKYBOX_EBO),
	_default_skybox(DEFAULT_CUBEMAP_PATHS),
	_default_texture(DEFAULT_TEXTURE_PATH)
{
	// Backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	// Depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);	// To not discard fragments with Z=1.0 in NDC (which is explicitly set with Skybox)
	// Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// GL_POINTS mode point size, set in vertex Shader
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::clear() const
{
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);	// TODO: Set back to black (aka comment out) to avoid the GL call
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// TODO: Make separate functions for each buffer clear?
}

void Renderer::draw_points(const Camera & camera, const std::vector<Mesh*>& meshes)
{
	// TODO
}

void Renderer::draw_meshes_unlit(const Camera& camera, const std::vector<Mesh*>& meshes)
{
	glUseProgram(_unlit_shader.get_ID());
	// Common Uniform data
	_unlit_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_unlit_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_unlit_shader.set_uniform_mat4("u_model_mat", glm::mat4(1.0));	// Individual meshes don't have a Transform, their VBO data is used as-is
	_unlit_shader.set_uniform_1i("u_diffuse_texture", TEX_SLOT_DIFFUSE);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	// Mesh-specific bindings
	for (const Mesh* mesh : meshes) {
		if (mesh->diffuse_tex && mesh->diffuse_tex->get_ID() != 0) 
			glBindTexture(GL_TEXTURE_2D, mesh->diffuse_tex->get_ID());
		else
			glBindTexture(GL_TEXTURE_2D, _default_texture.get_ID());
		glBindVertexArray(mesh->get_VAO());
		glDrawElements(GL_TRIANGLES, mesh->get_EBO_length(), GL_UNSIGNED_INT, 0);
	}
	// Leave no resources bound
	glBindTexture(GL_TEXTURE_2D, 0);
	// TODO: Other textures
	glBindVertexArray(0);
}

void Renderer::draw_unlit(const Camera& camera, const std::vector<Model*>& models)
{
	glUseProgram(_unlit_shader.get_ID());
	// Common Uniform data
	_unlit_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_unlit_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_unlit_shader.set_uniform_1i("u_diffuse_texture", TEX_SLOT_DIFFUSE);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	// Model-specific bindings
	for (const Model* model : models) {
		_unlit_shader.set_uniform_mat4("u_model_mat", model->get_model_mat());
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
	// TODO: Other textures
	glBindVertexArray(0);
}

void Renderer::draw_reflective(const Camera& camera, const std::vector<Model*>& models) { draw_reflective(camera, models, _default_skybox); }

void Renderer::draw_reflective(const Camera& camera, const std::vector<Model*>& models, const Cubemap& cubemap)
{
	glUseProgram(_reflective_shader.get_ID());

	_reflective_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_reflective_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_reflective_shader.set_uniform_vec3("u_camera_position", camera.get_position());
	_reflective_shader.set_uniform_1i("u_cubemap", TEX_SLOT_AMBIENT);
	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_AMBIENT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.get_ID());

	for (const Model* model : models) {
		_reflective_shader.set_uniform_mat4("u_model_mat", model->get_model_mat());
		_reflective_shader.set_uniform_mat3("u_normal_mat", model->get_normal_mat());
		for (const Mesh& mesh : model->meshes) {
			glBindVertexArray(mesh.get_VAO());
			glDrawElements(GL_TRIANGLES, mesh.get_EBO_length(), GL_UNSIGNED_INT, 0);
		}
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}

void Renderer::draw_skybox(const Camera& camera) { draw_skybox(camera, _default_skybox); }

void Renderer::draw_skybox(const Camera& camera, const Cubemap& skybox)
{
	glUseProgram(_skybox_shader.get_ID());
	//glDepthMask(GL_FALSE);	// Unnecessary with setting NDC Z=1.0 in vertex shader
	
	_skybox_shader.set_uniform_mat4("u_view_mat", camera.get_view_rotation_mat());
	_skybox_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_skybox_shader.set_uniform_1i("u_cubemap", TEX_SLOT_DIFFUSE);

	glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIFFUSE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.get_ID());
	glBindVertexArray(_skybox_cube.get_VAO());
	glDrawElements(GL_TRIANGLES, _skybox_cube.get_EBO_length(), GL_UNSIGNED_INT, 0);

	// Leave no resources bound and re-enable depth test
	//glDepthMask(GL_TRUE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}

void Renderer::draw_MPM_particles(const Camera& camera, const GLuint particlesVAO, const GLuint particlesNum, const glm::vec3 position)
{
	glUseProgram(_particle_shader.get_ID());
	
	_particle_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), position));	// Simulation can only be translated, NOT rotated/scaled
	_particle_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_particle_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	glBindVertexArray(particlesVAO);
	glDrawArrays(GL_POINTS, 0, particlesNum);

	glBindVertexArray(0);
}

void Renderer::draw_MPM_grid(const Camera& camera, const glm::vec3 position, const GLuint grid_VAO, const GLuint cells_num, const glm::uvec3& grid_size)
{
	glUseProgram(_grid_cell_shader.get_ID());

	_grid_cell_shader.set_uniform_mat4("u_model_mat", glm::translate(glm::mat4(1.0f), position));	// Simulation can only be translated, NOT rotated/scaled
	_grid_cell_shader.set_uniform_mat4("u_view_mat", camera.get_view_mat());
	_grid_cell_shader.set_uniform_mat4("u_projection_mat", camera.get_projection_mat());
	_grid_cell_shader.set_uniform_uvec3("u_grid_size", grid_size);
	glBindVertexArray(grid_VAO);
	glDrawArrays(GL_POINTS, 0, cells_num);

	glBindVertexArray(0);
}
