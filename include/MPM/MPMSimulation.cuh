#pragma once

#include <glad/glad.h>

#include <cuda.h>
#include <cuda_gl_interop.h>
#include <curand_kernel.h>

#define GLM_FORCE_CUDA
#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <MPM/ParticleMaterial.hpp>

class MPMSimulation
{
protected:

	float _timestep;
	glm::uvec3 _grid_size;
	unsigned int _particles_count;
	unsigned int _whitewater_count;
	unsigned int _whitewater_start_idx;	// Active whitewater are actually ping-ponged each frame between two halves of a buffer of size 2 * MAX_WHITEWATER_NUM
	float _water_level;
	// CUDA resources
	cudaGraphicsResource* _cells_velocities;
	cudaGraphicsResource* _cells_masses;
	cudaGraphicsResource* _particles_positions;
	cudaGraphicsResource* _particles_velocities;
	glm::aligned_mat3* _d_particles_velocity_gradients;
	curandState* _d_curand_states;	// cudaState for random particle initialization and whitewater spawn chance
	cudaGraphicsResource* _whitewater_positions;
	glm::aligned_vec3* _d_whitewater_velocities;
	cudaGraphicsResource* _whitewater_types;
	cudaGraphicsResource* _whitewater_lifetimes;
	unsigned int* _d_new_whitewater_counter;

	// OpenGL resources
	GLuint _cells_VAO; 
	GLuint _cells_velocities_VBO; 
	GLuint _cells_masses_VBO; 
	GLuint _particles_VAO; 
	GLuint _particles_positions_VBO; 
	GLuint _particles_velocities_VBO; 
	GLuint _whitewater_VAO; 
	GLuint _whitewater_positions_VBO;
	GLuint _whitewater_types_VBO;
	GLuint _whitewater_lifetimes_VBO;
	// Used in instanced rendering, not in simulation
	GLuint _quad_VBO; 
	glm::vec2 _quad_vertices[6] = {
		{-0.5f, -0.5f},
		{ 0.5f, -0.5f},
		{-0.5f,  0.5f},

		{ 0.5f, -0.5f},
		{ 0.5f,  0.5f},
		{-0.5f,  0.5f}
	};

	// Estimate water level at rest state (when reflections are more discernible), based on fluid properties and simulation dimension.
	void _estimate_water_level();

public:

	ParticleMaterial particles_material;
	float boundary;
	float boundary_elasticity;
	glm::aligned_vec3 gravity;
	glm::vec3 spawn_position;
	float whitewater_chance_min;
	float whitewater_chance_max;
	unsigned int whitewater_spawn_num;

	MPMSimulation(
		glm::uvec3 grid_size,
		const ParticleMaterial& particles_material,
		const float timestep,
		const float boundary = 0.0f,
		const float boundary_elasticity = 0.3f,
		const glm::aligned_vec3 gravity = glm::aligned_vec3(0.0f, -9.81f, 0.0f)
	);

	void cleanup();

	// Disable copy and move constructors and operators. They're not be used in this program, so they're disabled to ensure it and avoid problems with device resources management.
	MPMSimulation(const MPMSimulation&) = delete;
	MPMSimulation& operator=(const MPMSimulation&) = delete;
	MPMSimulation(MPMSimulation&&) = delete;
	MPMSimulation& operator=(MPMSimulation&&) = delete;

	GLuint get_cells_VAO() const;

	GLuint get_particles_VAO() const;

	GLuint get_whitewater_VAO() const;

	float get_timestep() const;

	glm::uvec3 get_grid_size() const;

	void set_grid_size(glm::uvec3 size);

	float get_estimated_water_level() const;

	unsigned int get_cells_count() const;

	unsigned int get_particles_count() const;

	unsigned int get_particles_max() const;

	bool can_spawn_particles() const;

	unsigned int get_whitewater_start_idx() const;

	unsigned int get_whitewater_count() const;

	unsigned int get_whitewater_max() const;

	void reset_simulation();

	void spawn_particles_sphere();

	void spawn_particles_cube();

	// The simulation always advances in steps of _timestep. If frametime il larger that _timestep, multiple iteration steps can be taken (set in main).
	void step();
};