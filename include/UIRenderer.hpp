#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

class UIRenderer
{
private:



public:

	UIRenderer(GLFWwindow* window);

	void new_frame() const;

	void render() const;

	void shutdown() const;

	void show_sample_ui() const;

	void show_FPS_counter(const unsigned int fps, const double avg_frametime) const;

	void show_simulation_info(const glm::uvec3 grid_size, const int particles_num) const;

	bool show_spawn_particle_circle_button() const;

	bool show_spawn_particle_square_button() const;

	void show_grid_rendering_checkbox(bool& show_grid) const;
};