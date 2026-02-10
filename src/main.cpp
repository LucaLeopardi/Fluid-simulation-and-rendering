#include <glad/glad.h>	// Must be included before GLFW
#include <GLFW/glfw3.h>
#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <iostream>
#include <memory>
#include <Camera.hpp>
#include <UIRenderer.hpp>
#include <Shader.hpp>
#include <Renderer.hpp>
#include <utils/Mesh.hpp>
#include <MPM/MPMSimulation.cuh>
#include <utils/deviceQuery.cuh>


#pragma region FUNCTION DECLARATIONS
// Debug context logger callback
void APIENTRY gl_debug_output(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam);

void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight);

void cursor_pos_callback(GLFWwindow* window, double cursorX, double cursorY);

void process_input(GLFWwindow* window, float delta_time);

#pragma endregion


///////////// MAIN /////////////

int window_width = 1920;
int window_height = 1080;
double last_cursor_x = 0.0;
double last_cursor_y = 0.0;
double cursor_dx = 0.0;
double cursor_dy = 0.0;
bool show_UI = true;
int prev_enter_key = GLFW_RELEASE;
// Data structure to use with glfwSetWindowUserPointer, to link custom data to the OpenGL window.
struct WindowUserPointer {
	std::unique_ptr<Camera> camera_u_ptr;
	std::unique_ptr<Renderer> renderer_u_ptr;
};

int main()
{
	runDeviceQuery();

	glfwInit();
	// Set minimum requirements to latest OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Maximize window
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	// Enable debugging in Debug CMake build preset
	#ifdef ENABLE_GL_DEBUG_CONTEXT
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif

	// Set up GLFW window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "GPUCRTGP", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	WindowUserPointer window_data;
	glfwSetWindowUserPointer(window, &window_data);	// To access data outside of main, from the OpenGL window

	// Set up glad
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize glad!" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	// Set up debug output
	int flags; 
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(gl_debug_output, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);	// Get debug messages from all sources, of all type, every severity 
	} 
	
	// Viewport and viewport resize callback setup
	glfwGetWindowSize(window, &window_width, &window_height);
	glfwSwapInterval(0);
	glViewport(0, 0, window_width, window_height);	// Same as GLFW window size
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	window_data.camera_u_ptr = std::make_unique<Camera>(window_width, window_height);
	glfwGetCursorPos(window, &last_cursor_x, &last_cursor_y);	// To avoid Camera jerk at first update
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	window_data.renderer_u_ptr = std::make_unique<Renderer>(window_width, window_height);	// Shaders are constructed in Renderer
	UIRenderer ui (window);

	glm::vec3 light_direction (-1.0f, 1.0f, -1.0f);


	//// Simulation initialization ////
	ParticleMaterial particle_material_water (
		125.0f,						// Mass (kg) represented by one Particle. MPM work best with at least 8 particles per grid Cell (density), so it was chosen accordingly.
		1000.0f,					// Rest density
		100.0f,						// Dynamic viscosity
		2000.0f,					// Eq. of State stiffness
		7.0f,						// Eq. of State power	
		-0.1f,						// Maximum negative pressure
		glm::vec3(0.1f, 0.0f, 0.9f)	// Color
	);
	glm::ivec3 grid_size (100, 80, 100);	// Minimum is 40x40x40
	MPMSimulation sim (
		grid_size,								// Simulation domain size. Set to at least 40 per dimension in ctor.
		particle_material_water,				// Material defining particle characteristics.
		0.017f, 								// Delta time for each simulation step (in seconds) (frametime ~17ms is 60 FPS)
		1.0f,									// Simulation boundary within to apply velocity dampening
		0.3f,									// Boundary wall "elasticity"
		glm::aligned_vec3(0.0f, -9.81f, 0.0f)	// Gravity acceleration
	);
	glm::vec3 sim_position (0.0f);
	glm::vec3 sim_center (sim_position + glm::vec3(sim.get_grid_size()) / 2.0f);

	window_data.camera_u_ptr->set_position(sim_center + glm::vec3(sim.get_grid_size().x * -0.75f, sim.get_grid_size().y * 0.25f, sim.get_grid_size().z * 0.75f));
	window_data.camera_u_ptr->set_direction(glm::vec3(sim_center.x, 0.0f, sim_center.z) - window_data.camera_u_ptr->get_position(), glm::vec3(0.0f, 1.0f, 0.0f));


	// Rendering options
	bool show_cubes = true;
	bool show_particles = true;
	bool show_grid = false;

	float sim_time_scale = 1.0f;
	bool sim_pause = false;
	float water_level = sim.get_estimated_water_level();


	#pragma region INITIAL SCENE SETUP
	
	for(int i = 1; i < grid_size.x / 20.0f; ++i) {
		for(int j = 1; j < grid_size.z / 20.0f; ++j) {
			sim.spawn_position = glm::vec3(20.0f * i, 10.0f, 20.0f * j);
			sim.spawn_particles_sphere();
			sim.spawn_position = glm::vec3(20.0f * i, 30.0f, 20.0f * j);
			sim.spawn_particles_sphere();
		} 
	} 
	sim.spawn_position = glm::vec3(sim.get_grid_size()) / 2.0f;

	// Spawn models
	std::vector<Model*> models;
	float models_speed = 40.0f;

	Model cube_green;
	cube_green.meshes.emplace_back(Mesh(
		{   // VBO
			// Position				// Normal				// Tex Coords	// Base color
			// Front
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 0
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 1
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 2
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 3
			// Right
			{{ 0.5f, -0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 4
			{{ 0.5f, -0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 5
			{{ 0.5f,  0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 6
			{{ 0.5f,  0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 7
			// Back
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 8
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 9
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 10
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 11
			// Left
			{{-0.5f, -0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 12
			{{-0.5f, -0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 13
			{{-0.5f,  0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 14
			{{-0.5f,  0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 15
			// Top
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 16
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 17
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 18
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 19
			// Bottom
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 20
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 21
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 22
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.8f, 0.1f, 1.0f}},	// 23
		},
		{   // EBO
				// Front
				0, 1, 2,
				3, 2, 1,
				// Right
				4, 5, 6,
				7, 6, 5,
				// Back
				8, 9,10,
				11,10, 9,
				// Left
				12,13,14,
				15,14,13,
				// Top
				16,17,18,
				19,18,17,
				// Bottom
				20,21,22,
				23,22,21,
			}
	));
	cube_green.set_position(sim_position + glm::vec3(0.0f, grid_size.y, 0.0f));
	cube_green.set_rotation({0.39f, 0.78f, 1.0f});
	cube_green.set_scale({5.0f, 5.0f, 5.0f});
	models.emplace_back(&cube_green);
	glm::vec3 red_target = sim_position + glm::vec3(0.0f, sim.get_grid_size().y + 10.0f, 0.0f);

	Model cube_red;
	cube_red.meshes.emplace_back(Mesh(
		{   // VBO
			// Position				// Normal				// Tex Coords	// Base color
			// Front
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 0
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 1
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 2
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 3
			// Right
			{{ 0.5f, -0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 4
			{{ 0.5f, -0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 5
			{{ 0.5f,  0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 6
			{{ 0.5f,  0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 7
			// Back
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 8
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 9
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 10
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 11
			// Left
			{{-0.5f, -0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 12
			{{-0.5f, -0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 13
			{{-0.5f,  0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 14
			{{-0.5f,  0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 15
			// Top
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 16
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 17
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 18
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 19
			// Bottom
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 20
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 21
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 22
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.8f, 0.1f, 0.1f, 1.0f}},	// 23
		},
		{   // EBO
				// Front
				0, 1, 2,
				3, 2, 1,
				// Right
				4, 5, 6,
				7, 6, 5,
				// Back
				8, 9,10,
				11,10, 9,
				// Left
				12,13,14,
				15,14,13,
				// Top
				16,17,18,
				19,18,17,
				// Bottom
				20,21,22,
				23,22,21,
			}
	));
	cube_red.set_position(sim_position + glm::vec3(-10.0f, sim_center.y, -10.0f));
	cube_red.set_scale({10.0f, 100.0f, 10.0f});
	cube_red.set_rotation({0.2f, 0.0f, 0.0f});
	models.emplace_back(&cube_red);
	glm::vec3 green_dir;

	Model cube_yellow;
	cube_yellow.meshes.emplace_back(Mesh(
		{   // VBO
			// Position				// Normal				// Tex Coords	// Base color
			// Front
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 0
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 1
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 2
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 3
			// Right
			{{ 0.5f, -0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 4
			{{ 0.5f, -0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 5
			{{ 0.5f,  0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 6
			{{ 0.5f,  0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 7
			// Back
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 8
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 9
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 10
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 11
			// Left
			{{-0.5f, -0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 12
			{{-0.5f, -0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 13
			{{-0.5f,  0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 14
			{{-0.5f,  0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 15
			// Top
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 16
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 17
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 18
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 19
			// Bottom
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 20
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 21
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 22
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.9f, 0.9f, 0.1f, 1.0f}},	// 23
		},
		{   // EBO
				// Front
				0, 1, 2,
				3, 2, 1,
				// Right
				4, 5, 6,
				7, 6, 5,
				// Back
				8, 9,10,
				11,10, 9,
				// Left
				12,13,14,
				15,14,13,
				// Top
				16,17,18,
				19,18,17,
				// Bottom
				20,21,22,
				23,22,21,
			}
	));
	cube_yellow.set_position(sim_position);
	cube_yellow.set_scale({10.0f, 10.0f, 10.0f});
	models.emplace_back(&cube_yellow);
	glm::vec3 yellow_dir;

	Model cube_blue;
	cube_blue.meshes.emplace_back(Mesh(
		{   // VBO
			// Position				// Normal				// Tex Coords	// Base color
			// Front
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 0
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 1
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 2
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  0.0f,  1.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 3
			// Right
			{{ 0.5f, -0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 4
			{{ 0.5f, -0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 5
			{{ 0.5f,  0.5f,  0.5f},	{ 1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 6
			{{ 0.5f,  0.5f, -0.5f},	{ 1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 7
			// Back
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 8
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 9
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 10
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  0.0f, -1.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 11
			// Left
			{{-0.5f, -0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 12
			{{-0.5f, -0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 13
			{{-0.5f,  0.5f, -0.5f},	{-1.0f,  0.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 14
			{{-0.5f,  0.5f,  0.5f},	{-1.0f,  0.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 15
			// Top
			{{-0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 16
			{{ 0.5f,  0.5f,  0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 17
			{{-0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 18
			{{ 0.5f,  0.5f, -0.5f},	{ 0.0f,  1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 19
			// Bottom
			{{-0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 20
			{{ 0.5f, -0.5f, -0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 0.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 21
			{{-0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{0.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 22
			{{ 0.5f, -0.5f,  0.5f},	{ 0.0f, -1.0f,  0.0f},	{1.0f, 1.0f}, 	{0.1f, 0.1f, 0.8f, 1.0f}},	// 23
		},
		{   // EBO
				// Front
				0, 1, 2,
				3, 2, 1,
				// Right
				4, 5, 6,
				7, 6, 5,
				// Back
				8, 9,10,
				11,10, 9,
				// Left
				12,13,14,
				15,14,13,
				// Top
				16,17,18,
				19,18,17,
				// Bottom
				20,21,22,
				23,22,21,
			}
	));
	cube_blue.set_position(sim_position + glm::vec3(sim.get_grid_size().x + 20.0f, 0.0f, 0.0f));
	cube_blue.set_scale({10.0f, 10.0f, 10.0f});
	models.emplace_back(&cube_blue);
	glm::vec3 blue_dir;
	
	#pragma endregion

	//// Rendering loop ////
	double delta_time = 0.0;
	double current_frame = glfwGetTime();
	double last_frame = current_frame;
	float sim_time_budget = 0.0;
	unsigned int sim_steps_this_frame = 0;
	const int MAX_SIMULATION_ITERATIONS = 1;	// To avoid "death spiral" of simulation trying to catch up to large frametime by taking multiple steps, and thus increasing frametime even more. Set to 1 since in this case the simulation itself is often the bottleneck.

	// Performance measuring vars
	unsigned int frames = 0;
	double frames_timer = 0.0;
	unsigned int fps = 0;
	double avg_frametime = 0.0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		if (!sim_pause) sim_time_budget += delta_time;																	// Compound delta_time if there was some left over in the previous frame
		sim_time_budget = std::min(sim_time_budget, MAX_SIMULATION_ITERATIONS * sim.get_timestep() / sim_time_scale);	// Clamp time budget according to time scale and max sim steps

		//// Simulation advancement ////
		/*
		The simulation always advances by a (small) fixed timestep for stability (aka avoid particle tunneling).
		If frametime is smaller, it waits until it compounds to the timestep to advance.
		If frametime is larger there are two possibilities:
		1. The simulation itself is the bottleneck: the simulation will "slow down". No solution, as having multiple iterations per frame to catch up would actually worsen the situation ("death spiral").
		2. Something else is the bottleneck: the simulation can advance by multiple iteration steps to catch up. To avoid "death spiral" of it increasing frametime exponentially, a max number of iterations is set.

		In other words: the simulation isn't slowed down only if frametime < timestep * MAX_ITERATIONS.
		However, if the simulation is the bottleneck, frametime grows linearly (?) with MAX_ITERATIONS. Increasing it then leads to a "death spiral".
		*/
		while (!sim_pause && sim_time_budget >= sim.get_timestep() / sim_time_scale) {
			sim.step();
			sim_time_budget -= sim.get_timestep() / sim_time_scale;
		}

		// Performance counter
		++frames;
		frames_timer += delta_time;
		if (frames_timer >= 1.0) {
			fps = frames;
			avg_frametime = (frames_timer / frames) * 1000;
			frames_timer = 0.0;
			frames = 0;
		}

		// Input processing
		process_input(window, delta_time);

		// Scene rendering
		if (window_data.camera_u_ptr && window_data.renderer_u_ptr) {
			Camera* camera = window_data.camera_u_ptr.get();
			Renderer* renderer = window_data.renderer_u_ptr.get();
			renderer->clear();
			// Draw opaque geometry
			if (show_cubes) renderer->draw_lit_textured(*camera, light_direction, &models, false);
			// Draw skybox
			renderer->draw_skybox(*camera);	// Drawn AFTER the opaque models to optimize if covered
			// Draw transparent objects (fluid)
			// Lerp water level to ease reflections POV change
			float water_level_delta = sim.get_estimated_water_level() - water_level;
			if (abs(water_level_delta) > 0.01f) water_level += water_level_delta * delta_time;
			if (show_particles) {
				if (renderer->particles_rendering_mode == PARTICLES_RENDERING::FLUID) {
					renderer->draw_fluid(
						*camera,						// Camera
						sim.get_particles_VAO(),		// Sim. particles data
						sim.get_particles_count(),		// Sim. particles count
						sim.get_whitewater_VAO(),		// Sim. whitewater data
						sim.get_whitewater_start_idx(),	// Sim. whitewater buffer start index
						sim.get_whitewater_count(),		// Sim. whitewater count
						sim_position,					// Sim. origin world position
						sim_center,						// Sim. center world position
						water_level,					// Sim. estimated water level (for reflections)
						show_cubes ? &models : nullptr,	// Reflected models, null for no dynamic reflections
						false,							// Whether reflected models are to be drawn textured 
						light_direction					// Scene light direction to render reflections
					);
				} else {
					renderer->draw_MPM_particles(
						*camera,
						sim.get_particles_VAO(), 
						sim.get_particles_count(), 
						sim_position, 
						light_direction
					);
				}
			}
			// Draw MPM grid
			if (show_grid) renderer->draw_MPM_grid(
				*camera, 
				sim_position, 
				sim.get_cells_VAO(), 
				sim.get_cells_count(), 
				sim.get_grid_size()
			);
		}

		// Move cubes around
		if (show_cubes) {
			sim_center = glm::vec3(sim.get_grid_size()) / 2.0f;
			// Green
			if (cube_green.get_position().x >= sim_position.x + grid_size.x || cube_green.get_position().z >= sim_position.z + grid_size.z) 
				red_target = sim_position + glm::vec3(0.0f, sim.get_grid_size().y, 0.0f);
			if (cube_green.get_position().x <= sim_position.x || cube_green.get_position().z <= sim_position.z) 
				red_target = sim_position + glm::vec3(sim.get_grid_size());
			cube_green.translate((float) delta_time * models_speed * normalize(red_target - cube_green.get_position()));
			// Red
			glm::vec3 p = glm::vec3(cube_red.get_position().x, 0.0f, cube_red.get_position().z);
			glm::vec3 c = glm::vec3(sim_center.x, 0.0f, sim_center.z);
			float r = length(glm::vec3(sim_position.x, 0.0f, sim_position.z) - c) + 40.0f;
			green_dir = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(p - c));							// Move along tangent
			green_dir += glm::normalize(p - c) * r - (p - c);													// Move to new radius
			//green_dir += glm::vec3(0.0f, (sim.get_grid_size().y + 10.0f) - cube_red.get_position().y, 0.0f);	// Move to new Y
			if (length(green_dir) > 0.0f) green_dir = normalize(green_dir);
			cube_red.translate((float) delta_time * models_speed * green_dir);
			// Yellow
			yellow_dir = glm::normalize(cube_yellow.get_position() - sim_center) * (length(sim_position - sim_center)) - (cube_yellow.get_position() - sim_center);	// Move to new radius
			yellow_dir += glm::cross(glm::vec3(0.33f, 0.33f, -0.33f), glm::normalize(cube_yellow.get_position() - sim_center));			// Move along tangent
			if (length(yellow_dir) > 0.0f) yellow_dir = normalize(yellow_dir);
			cube_yellow.translate((float) delta_time * models_speed * yellow_dir);
			// Blue
			blue_dir = glm::normalize(cube_blue.get_position() - sim_center) * (length(sim_position - sim_center)) - (cube_blue.get_position() - sim_center);	// Move to new radius
			blue_dir += glm::cross(glm::vec3(-0.33f, 0.33f, -0.33f), glm::normalize(cube_blue.get_position() - sim_center));			// Move along tangent
			if (length(blue_dir) > 0.0f) blue_dir = normalize(blue_dir);
			cube_blue.translate((float) delta_time * models_speed * blue_dir);
		}

		// UI
		ui.show_controls();
		if (show_UI) {
			ui.ui_frame();
			ui.show_time_buttons(sim_pause, sim_time_scale);
			ui.show_FPS_counter(fps, avg_frametime, (sim.get_timestep() / sim_time_scale) * 1000.0f);
			ui.show_simulation_info(sim.get_grid_size(), sim.get_particles_count(), sim.get_particles_max(), sim.get_whitewater_count(), sim.get_whitewater_max());
			if (ui.show_reset_simulation_button()) sim.reset_simulation();
			if (ui.show_grid_settings(grid_size, sim.boundary, sim.boundary_elasticity)) sim.set_grid_size(grid_size);
			ui.show_fluid_properties(
				sim.particles_material.dynamic_viscosity, 
				sim.particles_material.EOS_stiffness, 
				sim.particles_material.max_negative_pressure,
				sim.whitewater_chance_min,
				sim.whitewater_chance_max,
				sim.whitewater_spawn_num);
			ui.show_spawn_position_settings(sim.spawn_position, sim.get_grid_size());
			if (ui.show_spawn_particle_sphere_button()) sim.spawn_particles_sphere();
			if (ui.show_spawn_particle_cube_button(sim.can_spawn_particles())) sim.spawn_particles_cube();
			if (window_data.renderer_u_ptr)ui.show_rendering_settings(show_cubes, show_particles, show_grid, *(window_data.renderer_u_ptr));
			ui.end_frame();
		}
		ui.render();

		glfwSwapBuffers(window);
	}
	
	sim.cleanup();
	ui.shutdown();
	glfwTerminate();
	return 0;
}


#pragma region FUNCTION DEFINITIONS

// Debug context logger callback
void APIENTRY gl_debug_output(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "\n---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification"; break;
    } 
	std::cout << std::endl;
    std::cout << std::endl;
	// ^ Set debugger breakpoint here  to get stack trace as soon as the error prints
}


void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	// Avoids glm crash with viewport dimensions 0. In practice, ignores window minimization.  
	if (newWidth <= 0 || newHeight <= 0) return;

	window_width = newWidth;
	window_height = newHeight;
	glViewport(0, 0, window_width, window_height);
	WindowUserPointer* window_data = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(window));
	if (window_data)
	{
		// Update Camera if it has been set
		if (window_data->camera_u_ptr) window_data->camera_u_ptr->set_viewport_size(window_width, window_height);
		// Update Renderer's particles target texture
		if (window_data->renderer_u_ptr) window_data->renderer_u_ptr->on_framebuffer_size_change(window_width, window_height);
	}
}

void cursor_pos_callback(GLFWwindow* window, double cursorX, double cursorY)
{
	cursor_dx = last_cursor_x - cursorX;
	cursor_dy = last_cursor_y - cursorY;
	last_cursor_x = cursorX;
	last_cursor_y = cursorY;
}

void process_input(GLFWwindow* window, float delta_time)
{
	// Close app
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
	// Show/hide UI
	int enter_key = glfwGetKey(window, GLFW_KEY_ENTER);
	if (enter_key == GLFW_PRESS && prev_enter_key == GLFW_RELEASE) show_UI = !show_UI;
	prev_enter_key = enter_key;

	// Update Camera if it has been set
	WindowUserPointer* window_data = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(window));
	if (window_data && window_data->camera_u_ptr) {
		// Camera keyboard movement
		glm::vec3 cameraMovementVector (0.0f);
		bool fasterMovement = false;
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraMovementVector.z += 1.0f;
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraMovementVector.z -= 1.0f;
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraMovementVector.x += 1.0f;
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraMovementVector.x -= 1.0f;
		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraMovementVector.y += 1.0f;
		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cameraMovementVector.y -= 1.0f;
		if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) fasterMovement = true;
		if (glm::length(cameraMovementVector) > 0.0f) {
			cameraMovementVector = glm::normalize(cameraMovementVector);	// Direction Camera speed will be applied on
			window_data->camera_u_ptr->move(cameraMovementVector, fasterMovement, (float) delta_time);
		}
		// Camera mouse movement
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && (cursor_dx != 0.0 || cursor_dy != 0.0)) {
			window_data->camera_u_ptr->rotate(cursor_dx, cursor_dy);	// Cursor delta is used as yaw/pitch rotation (in radians), Camera sensitivity will be applied
			// Consume last mouse delta because cursor callback isn't called when it stops moving
			cursor_dx = 0.0;
			cursor_dy = 0.0;
		}
	}
}

#pragma endregion 