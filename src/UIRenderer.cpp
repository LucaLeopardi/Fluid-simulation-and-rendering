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

void UIRenderer::new_frame() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// TODO: Change this if multiple panels are necessary
	ImGui::Begin("UI", NULL, 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove
	);	
}

void UIRenderer::render() const
{
	ImGui::End();	// TODO: Change this if multiple panels are necessary
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIRenderer::shutdown() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UIRenderer::show_sample_ui() const { ImGui::ShowDemoWindow(); }

void UIRenderer::show_FPS_counter(const unsigned int fps, const double avg_frametime) const
{
	ImGui::BeginChild("Performance", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Text("FPS: %i", fps);
	ImGui::Text("Avg. frametime: %.4f ms", avg_frametime);
	ImGui::EndChild();
}

void UIRenderer::show_simulation_info(const glm::uvec3 grid_size, const int particles_num) const
{
	ImGui::BeginChild("Simulation info", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Text("Grid size: %ix%ix%i", grid_size.x, grid_size.y, grid_size.z);
	ImGui::Text("Particles: %i", particles_num);
	ImGui::EndChild();
}

bool UIRenderer::show_spawn_particle_circle_button() const
{
	ImGui::BeginChild("Particles spawn", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	bool result = ImGui::Button("Spawn particles circle");
	ImGui::EndChild();
	
	return result;
}

bool UIRenderer::show_spawn_particle_square_button() const
{
	ImGui::BeginChild("Particles spawn", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	bool result = ImGui::Button("Spawn particles square");
	ImGui::EndChild();
	
	return result;
}

void UIRenderer::show_grid_rendering_checkbox(bool & show_grid) const
{
	ImGui::BeginChild("Grid rendering", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
	ImGui::Checkbox("Show grid", &show_grid);
	ImGui::EndChild();
}
