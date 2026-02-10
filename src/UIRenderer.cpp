#include <UIRenderer.hpp>
#include <string>

UIRenderer::UIRenderer(GLFWwindow * window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = NULL; 	// Disable saving settings and UI state
	ImGui::StyleColorsDark();
	
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void UIRenderer::show_controls() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::Begin("Controls", NULL, 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground
	);
	ImGui::TextColored(
		ImVec4(1.0, 1.0, 1.0, 0.6), 
		"RIGHT MOUSE: Rotate camera	\
		WASD: Move camera horizontally	\
		E/Q or SPACE/CTRL: Move camera up/down	\
		SHIFT: Fast camera movement	\
		ENTER: Toggle UI	\
		ESC: Exit"
	);
	ImGui::End();
}

void UIRenderer::ui_frame() const
{
	ImGui::SetNextWindowBgAlpha(0.7f);
	ImGui::Begin("UI", NULL,
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove
	);	
}

void UIRenderer::end_frame() const
{
	ImGui::End();
}

void UIRenderer::render() const
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIRenderer::shutdown() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UIRenderer::same_line() const
{
	ImGui::SameLine();
}

void UIRenderer::show_sample_ui() const { ImGui::ShowDemoWindow(); }

void UIRenderer::show_FPS_counter(const unsigned int fps, const double avg_frametime, const float target_frametime) const
{
	ImGui::BeginChild("Performance", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Text("FPS: %i", fps);
	ImGui::TextColored(avg_frametime <= target_frametime ? ImVec4(1,1,1,1) : ImVec4(1,0,0,1), "Avg. frametime: %.4f ms\nSimulation req.: < %.1f ms", avg_frametime, target_frametime);
	ImGui::EndChild();
}

void UIRenderer::show_simulation_info(const glm::uvec3 grid_size, const int particles_num, const int particles_max, const int whitewater_num, const int whitewater_max) const
{
	ImGui::BeginChild("Simulation info", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Text("Grid size:	%ix%ix%i", grid_size.x, grid_size.y, grid_size.z);
	ImGui::Text("Particles:	%i", particles_num);
	ImGui::SameLine();
	ImGui::TextColored(particles_num < particles_max ? ImVec4(1,1,1,1) : ImVec4(1,0,0,1), "	(Max: %i)", particles_max);
	ImGui::Text("Whitewater:	%i", whitewater_num);
	ImGui::SameLine();
	ImGui::TextColored(whitewater_num < whitewater_max ? ImVec4(1,1,1,1) : ImVec4(1,1,0,1), "	(Max: %i)", whitewater_max);
	ImGui::EndChild();
}

void UIRenderer::show_time_buttons(bool& pause, float& time_scale) const
{
	ImGui::BeginChild("Time settings", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	if (ImGui::Button(pause ? "Start" : "Pause", ImVec2(120.0f, 20.0f))) pause = !pause;
	
	ImGui::SameLine();
	ImGui::BeginDisabled(time_scale == 0.1f);
	if (ImGui::Button("x0.1", ImVec2(50.0f, 20.0f))) time_scale = 0.1f;
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(time_scale == 0.5f);
	if (ImGui::Button("x0.5", ImVec2(50.0f, 20.0f))) time_scale = 0.5f;
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(time_scale == 1.0f);
	if (ImGui::Button("x1.0", ImVec2(50.0f, 20.0f))) time_scale = 1.0f;
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(time_scale == 1.5f);
	if (ImGui::Button("x1.5", ImVec2(50.0f, 20.0f))) time_scale = 1.5f;
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(time_scale == 2.0f);
	if (ImGui::Button("x2.0", ImVec2(50.0f, 20.0f))) time_scale = 2.0f;
	ImGui::EndDisabled();
	
	ImGui::EndChild();
}

bool UIRenderer::show_reset_simulation_button() const
{
	ImGui::BeginChild("Simulation reset", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	bool clicked = ImGui::Button("Reset simulation", ImVec2(300.0f, 20.0f));
	ImGui::EndChild();

	return clicked;
}

void UIRenderer::show_spawn_position_settings(glm::vec3& spawn_position, const glm::ivec3 grid_size ) const
{
	ImGui::BeginChild("Particles spawn", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::SetNextItemWidth(64);
	ImGui::SliderFloat("##X", &spawn_position.x, 20.0f, grid_size.x - 20.0f, "X: %.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(64);
	ImGui::SliderFloat("##Y", &spawn_position.y, 20.0f, grid_size.y - 20.0f, "Y: %.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(64);
	ImGui::SliderFloat("##Z", &spawn_position.z, 20.0f, grid_size.z - 20.0f, "Z: %.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine(0, 5.0f);
	ImGui::Text("Spawn position");
	//ImGui::EndChild();
}

bool UIRenderer::show_spawn_particle_sphere_button(const bool enabled) const
{
	//ImGui::BeginChild("Particles spawn", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::BeginDisabled(!enabled);
	bool clicked = ImGui::Button("Spawn particles (sphere)", ImVec2(300.0f, 20.0f));
	ImGui::EndDisabled();
	//ImGui::EndChild();
	
	return clicked;
}

bool UIRenderer::show_spawn_particle_cube_button(const bool enabled) const
{
	//ImGui::BeginChild("Particles spawn cube", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::BeginDisabled(!enabled);
	bool clicked = ImGui::Button("Spawn particles (cube)", ImVec2(300.0f, 20.0f));
	ImGui::EndDisabled();
	ImGui::EndChild();
	
	return clicked;
}

void UIRenderer::show_fluid_properties(
	float& viscosity, 
	float& stiffness, 
	float& max_neg_pressure, 
	float& whitewater_chance_min,
	float& whitewater_chance_max,
	unsigned int& whitewater_spawn_num
) const
{
	ImGui::BeginChild("Fluid properties", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Separator();
	ImGui::Text("MPM particles settings");
	ImGui::Separator();
	ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 300.0f, "%.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SliderFloat("Stiffness", &stiffness, 0.0f, 3000.0f, "%.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SliderFloat("Max negative pressure", &max_neg_pressure, -10.0f, 0.0f, "%.1f", ImGuiSliderFlags_ClampOnInput);
	ImGui::DragFloatRange2("Whitewater threshold", &whitewater_chance_min, &whitewater_chance_max, 0.05f, 0.00f, 5.00f, "%.2f", "%.2f", ImGuiSliderFlags_AlwaysClamp);
	unsigned int spawn_min = 0;
	unsigned int spawn_max = 40;
	ImGui::SliderScalar("Whitewater max spawn", ImGuiDataType_U32, &whitewater_spawn_num, &spawn_min, &spawn_max, "%u", ImGuiSliderFlags_ClampOnInput);
	ImGui::EndChild();
}

bool UIRenderer::show_grid_settings(glm::ivec3& size, float& boundary, float& boundary_elasticity) const
{
	bool changed_grid = false;
	ImGui::BeginChild("Grid settings", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Separator();
	ImGui::Text("MPM grid settings");
	ImGui::Separator();
	ImGui::SetNextItemWidth(64);
	changed_grid = changed_grid || ImGui::SliderInt("##X", &size.x, 32.0f, 200.0f, "X: %d", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(64);
	changed_grid = changed_grid || ImGui::SliderInt("##Y", &size.y, 32.0f, 200.0f, "Y: %d", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(64);
	changed_grid = changed_grid || ImGui::SliderInt("##Z", &size.z, 32.0f, 200.0f, "Z: %d", ImGuiSliderFlags_ClampOnInput);
	ImGui::SameLine(0, 5.0f);
	ImGui::Text("Size");
	
	ImGui::SliderFloat("Boundary size", &boundary, 0.0f, 6.0f, "%.0f", ImGuiSliderFlags_ClampOnInput);
	ImGui::SliderFloat("Boundary elasticity", &boundary_elasticity, 0.0f, 1.0f, "%.1f", ImGuiSliderFlags_ClampOnInput);
	ImGui::EndChild();

	return changed_grid;
}

const char* skyboxes_names[] = 
{
	"Moonrise",
	"Dusk",
	"Hills",
	"Alien cave",
	"Fantasy sky",
};
const char* rendering_modes_names[] = 
{
	"Point particles",
	"Sphere particles",
	"Fluid"
};
const char* particles_color_names[] =
{
	"Velocity",
	"Velocity components",
	"Diffuse"
};
const char* fluid_passes_names[] =
{
	"Thickness",
	"Depth",
	"Normals",
	"Whitewater",
	"Final"
};
void UIRenderer::show_rendering_settings(bool& show_cubes, bool& show_particles, bool& show_grid, Renderer& renderer) const
{
	ImGui::BeginChild("Rendering settings", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Separator();
	ImGui::Text("Rendering settings");
	ImGui::Separator();

	int skybox_idx = static_cast<int>(renderer.skybox_idx);
	if (ImGui::Combo("Skybox", &skybox_idx, skyboxes_names, IM_ARRAYSIZE(skyboxes_names)))
		renderer.skybox_idx = static_cast<SKYBOX>(skybox_idx);

	ImGui::Checkbox("Show cubes", &show_cubes);
	ImGui::Checkbox("Show particles/fluid", &show_particles);
	ImGui::Checkbox("Show whitewater", &renderer.fluid_show_whitewater);
	ImGui::Checkbox("Show MPM grid", &show_grid);

	int rendering_mode_idx = static_cast<int>(renderer.particles_rendering_mode);
	if (ImGui::Combo("Rendering mode", &rendering_mode_idx, rendering_modes_names, IM_ARRAYSIZE(rendering_modes_names)))
		renderer.particles_rendering_mode = static_cast<PARTICLES_RENDERING>(rendering_mode_idx);


	if (renderer.particles_rendering_mode == POINTS || renderer.particles_rendering_mode == QUADS) {
		int particles_color_idx = static_cast<int>(renderer.particles_color);
		int colors_size = IM_ARRAYSIZE(particles_color_names);
		if (renderer.particles_rendering_mode == POINTS) {
			colors_size = 2;
			if (particles_color_idx >= colors_size) {
				renderer.particles_color = VELOCITY;
				particles_color_idx = static_cast<int>(renderer.particles_color);
			} 
		}
		if (ImGui::Combo("Particles color", &particles_color_idx, particles_color_names, renderer.particles_rendering_mode == POINTS ? 2 : IM_ARRAYSIZE(particles_color_names)))
			renderer.particles_color = static_cast<PARTICLES_COLOR>(particles_color_idx);
		ImGui::SliderFloat("Particles size", &renderer.particles_radius, 0.1f, 1.0f, "%.1f");
	}

	if (renderer.particles_rendering_mode == FLUID) {
		int fluid_pass_idx = static_cast<int>(renderer.fluid_pass);
		if (ImGui::Combo("Render pass", &fluid_pass_idx, fluid_passes_names, IM_ARRAYSIZE(fluid_passes_names)))
			renderer.fluid_pass = static_cast<FLUID_PASS>(fluid_pass_idx);

		ImGui::SliderFloat("Particles size", &renderer.particles_radius, 0.1f, 1.0f, "%.1f");
		ImGui::SliderFloat("Particle thickness", &renderer.particle_thickness, 0.0f, 0.1f, "%.3f");
		ImGui::SliderFloat("Refraction", &renderer.fluid_refraction_strength, 0.001f, 0.1f, "%.3f");
		ImGui::SetNextItemWidth(64);
		ImGui::SliderFloat("##R", &renderer.fluid_light_absorbance.r, 0.0f, 1.0f, "R: %.2f", ImGuiSliderFlags_ClampOnInput);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(64);
		ImGui::SliderFloat("##G", &renderer.fluid_light_absorbance.g, 0.0f, 1.0f, "G: %.2f", ImGuiSliderFlags_ClampOnInput);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(64);
		ImGui::SliderFloat("##B", &renderer.fluid_light_absorbance.b, 0.0f, 1.0f, "B: %.2f", ImGuiSliderFlags_ClampOnInput);
		ImGui::SameLine(0, 5.0f);
		ImGui::Text("Light absorbance");
		ImGui::SliderFloat("Fresnel pow.", &renderer.fluid_fresnel_pow, 0.0f, 10.0f, "%.1f");

		ImGui::SliderInt("Blur passes", &renderer.fluid_depth_blur_passes, 0, 10);
		ImGui::SliderFloat("Blur kernel size", &renderer.fluid_blur_radius, 0.0f, 0.5f, "%.3f");
		ImGui::SliderFloat("Blur smoothness", &renderer.fluid_blur_smoothness, 0.01f, 10.0f, "%.2f");
		ImGui::SliderFloat("Blur depth", &renderer.fluid_blur_depth, 0.01f, 10.0f, "%.2f");
	}

	ImGui::EndChild();
}
