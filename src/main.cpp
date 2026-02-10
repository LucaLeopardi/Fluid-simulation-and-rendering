#include <glad/glad.h>	// Must be included before GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <Camera.hpp>
#include <UIRenderer.hpp>
#include <Shader.hpp>
#include <Renderer.hpp>
#include <utils/Mesh.hpp>
#include <MPM/MPMSimulation.hpp>

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

int window_width = 1000;
int window_height = 800;
double last_cursor_x = 0.0;
double last_cursor_y = 0.0;
double cursor_dx = 0.0;
double cursor_dy = 0.0;
// Data structure to use with glfwSetWindowUserPointer, to link custom data to the OpenGL window.
struct WindowUserPointer {
	std::unique_ptr<Camera> camera_u_ptr;
};

int main()
{
	glfwInit();
	// Set minimum requirements to latest OpenGL version. 4.6 shouldn't be a problem since it's from 2017?
	// Still, might want to lower it to increase compatibility.
	// Minimum for GL DEBUG CONTEXT: 4.3
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
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "TestOpenGL", NULL, NULL);
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
	glViewport(0, 0, window_width, window_height);	// Same as GLFW window size
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// TODO: HDR and bloom?
	// TODO: Gamma correction?
	// TODO: Antialiasing?


	window_data.camera_u_ptr = std::make_unique<Camera>(window_width, window_height);
	glfwGetCursorPos(window, &last_cursor_x, &last_cursor_y);	// To avoid Camera jerk at first update
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	UIRenderer ui (window);

	Renderer renderer = Renderer();	// Shaders constructed in Renderer

	//// Test simulation ////
	const int MAX_SIMULATION_ITERATIONS = 1;	// To avoid "death spiral" of simulation trying to catch up to large frametime by taking multiple steps, and thus increasing frametime even more. Set to 1 since in this case the simulation itself is the bottleneck.
	const glm::uvec3 grid_size (100, 100, 100);	// Each dimension must be greater than 5 to even function. At least 16 to show simulation behaviour.
	MPMSimulation sim (
		grid_size,						// Simulation domain size. Set to at least 6 per dimension in ctor.
		0.016f, 							// Delta time for each simulation step
		125.0f,							// Mass (kg) represented by one Particle. MPM work best with at least 8 particles per grid Cell, so choose accordingly with ParticleMaterial density.
		0.0f,							// Simulation boundary within to apply velocity dampening
		0.3f,							// Boundary wall "elasticity"
		glm::vec3(0.0f, -9.81f, 0.0f)	// Gravity acceleration
	);
	ParticleMaterial particle_material_water (
		1000.0f,					// Rest density
		100.0f,						// Dynamic viscosity
		2000.0f,					// Eq. of State stiffness
		7.0f,						// Eq. of State power	
		glm::vec3(0.1f, 0.0f, 0.9f)	// Color
	);
	glm::vec3 sim_position (-(grid_size.x / 2.0f) + 2.0f, -(grid_size.y / 2.0f), grid_size.z * (-1.5f));

	// OpenGL rendering resources
	GLuint particles_VBO, particles_VAO;
	
	glGenBuffers(1, &particles_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
	glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);	// Could be skipped actually, since it should be empty at this time. Particles are spawned later.

	glGenVertexArrays(1, &particles_VAO);
	glBindVertexArray(particles_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*) offsetof(Particle, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*) offsetof(Particle, velocity));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*) offsetof(Particle, mass));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*) offsetof(Particle, volume));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*) (offsetof(Particle, material) + offsetof(ParticleMaterial, color)));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	GLuint grid_VBO, grid_VAO;

	glGenBuffers(1, &grid_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sim.get_cells_count() * sizeof(Cell), &(sim.get_grid()[0]), GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Cell), (void*) offsetof(Cell, velocity));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Cell), (void*) offsetof(Cell, mass));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	bool show_grid = false;
	

	//// Rendering loop ////
	double delta_time = 0.0;
	double last_frame = glfwGetTime();
	double sim_time_budget = 0.0;
	unsigned int sim_steps_this_frame = 0;

	// Temp frame counter before UI is implemented
	unsigned int frames = 0;
	double frames_timer = 0.0;
	unsigned int fps = 0;
	double avg_frametime = 0.0;
	int sim_steps_elapsed = 0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		delta_time = glfwGetTime() - last_frame;
		last_frame = glfwGetTime();
		sim_time_budget += delta_time;	// Compounds delta_time if there was some left over in the previous frame
		sim_steps_this_frame = 0;

		/// Test simulation ////
		/*
		The simulation always advances by a (small) fixed timestep for stability (aka avoid particle tunneling).
		If frametime is smaller, it waits until it compounds to the timestep to advance.
		If frametime is larger there are two possibilities:
		1. The simulation itself is the bottleneck: the simulation will "slow down". No solution, as having multiple iterations per frame to catch up would actually worsen the situation ("death spiral").
		2. Something else is the bottleneck: the simulation can advance by multiple iteration steps to catch up. To avoid "death spiral" of it increasing frametime exponentially, a max number of iterations is set.

		In other words: the simulation isn't slowed down only if frametime < timestep * MAX_ITERATIONS.
		However, if the simulation is the bottleneck, frametime grows linearly (?) with MAX_ITERATIONS. Increasing it then leads to a "death spiral".
		*/
		while (sim_time_budget >= sim.get_timestep() && sim_steps_this_frame < MAX_SIMULATION_ITERATIONS) {
			sim.step();
			++sim_steps_this_frame;
			sim_time_budget -= sim.get_timestep();
		}
		if (sim_steps_this_frame > 0 && sim.get_particles_count() > 0) {	// Only update data if the simulation advanced 
			glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);					// Orphan old data
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), &(sim.get_particles()[0]), GL_DYNAMIC_DRAW);	// Set new data
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			if (show_grid) {
				glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
				glBufferData(GL_ARRAY_BUFFER, sim.get_cells_count() * sizeof(Cell), nullptr, GL_DYNAMIC_DRAW);			// Orphan old data
				glBufferData(GL_ARRAY_BUFFER, sim.get_cells_count() * sizeof(Cell), &(sim.get_grid()[0]), GL_DYNAMIC_DRAW);	// Set new data
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		// FPS counter
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
		if (window_data.camera_u_ptr) {
			renderer.clear();
			if (show_grid) renderer.draw_MPM_grid(*(window_data.camera_u_ptr), sim_position, grid_VAO, sim.get_cells_count(), sim.get_grid_size());
			renderer.draw_MPM_particles(*(window_data.camera_u_ptr), particles_VAO, sim.get_particles_count(), sim_position);
			//renderer.draw_skybox(*(window_data.camera_u_ptr));			// Drawn AFTER the models to optimize if covered
		}

		// UI
		ui.new_frame();
		ui.show_FPS_counter(fps, avg_frametime);
		ui.show_simulation_info(sim.get_grid_size(), sim.get_particles_count());
		if (ui.show_spawn_particle_circle_button()) {
			sim.spawn_particles_sphere(16 * 16 * 16, particle_material_water);
			glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);						// Orphan old data
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), &(sim.get_particles()[0]), GL_DYNAMIC_DRAW);	// Set new data
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		if (ui.show_spawn_particle_square_button()) {
			sim.spawn_particles_cube(16 * 16 * 16, particle_material_water);
			glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);						// Orphan old data
			glBufferData(GL_ARRAY_BUFFER, sim.get_particles_count() * sizeof(Particle), &(sim.get_particles()[0]), GL_DYNAMIC_DRAW);	// Set new data
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		ui.show_grid_rendering_checkbox(show_grid);
		ui.render();

		glfwSwapBuffers(window);
	}
	
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
	// Update Camera if it has been set
	WindowUserPointer* window_data = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(window));
	if (window_data && window_data->camera_u_ptr) window_data->camera_u_ptr->set_viewport_size(window_width, window_height);
}

void cursor_pos_callback(GLFWwindow* window, double cursorX, double cursorY)
{
	cursor_dx = last_cursor_x - cursorX;
	cursor_dy = last_cursor_y - cursorY;
	last_cursor_x = cursorX;
	last_cursor_y = cursorY;
}

// TODO: consider key callbacks instead
void process_input(GLFWwindow* window, float delta_time)
{
	// Close app
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
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
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraMovementVector.y += 1.0f;
		if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cameraMovementVector.y -= 1.0f;
		if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) fasterMovement = true;
		if (glm::length(cameraMovementVector) > 0.0f) {
			cameraMovementVector = glm::normalize(cameraMovementVector);	// Direction, Camera speed will be applied
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