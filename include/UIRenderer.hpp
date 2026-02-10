#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <Renderer.hpp>

class UIRenderer
{
private:



public:

	UIRenderer(GLFWwindow* window);

	void show_controls() const;

	void ui_frame() const;

	void end_frame() const;

	void render() const;

	void shutdown() const;

	void same_line() const;

	void show_sample_ui() const;

	void show_FPS_counter(const unsigned int fps, const double avg_frametime, const float target_frametime) const;

	void show_simulation_info(const glm::uvec3 grid_size, const int particles_num, const int particles_max, const int whitewater_num, const int whitewater_max) const;

	void show_time_buttons(bool& pause, float& time_scale) const;

	bool show_reset_simulation_button() const;

	void show_spawn_position_settings(glm::vec3& spawn_position, const glm::ivec3 grid_size) const;

	bool show_spawn_particle_sphere_button(const bool enabled = true) const;

	bool show_spawn_particle_cube_button(const bool enabled = true) const;

	void show_fluid_properties(
		float& viscosity, 
		float& stiffness, 
		float& max_neg_pressure, 
		float& whitewater_chance_min,
		float& whitewater_chance_max,
		unsigned int& whitewater_spawn_num
	) const;

	bool show_grid_settings(glm::ivec3& size, float& boundary, float& boundary_elasticity) const;

	void show_rendering_settings(bool& show_cubes, bool& show_particles, bool& show_grid, Renderer& renderer) const;
};