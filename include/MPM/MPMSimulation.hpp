#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <MPM/Particle.hpp>
#include <MPM/ParticleMaterial.hpp>
#include <MPM/Cell.hpp>

class MPMSimulation
{
private:
	glm::uvec3 _grid_size;
	std::vector<Cell> _grid;
	std::vector<Particle> _particles;
	float _particle_mass;
	float _timestep;
	float _boundary;
	float _boundary_elasticity;

public:

	float max_initial_speed;
	glm::vec3 gravity;

	MPMSimulation(
		glm::uvec3 grid_size,
		const float timestep,
		const float _particle_mass,
		const float boundary = 1.0f,
		const float boundary_elasticity = 0.3f,
		const glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f)
		);
		
	float get_timestep() const;

	unsigned int get_cells_count() const;
		
	const std::vector<Cell>& get_grid() const;
		
	glm::uvec3 get_grid_size() const;

	Cell& get_cell(unsigned int x, unsigned int y, unsigned int z);

	Cell& get_cell(const glm::uvec3& idx);

	const std::vector<Particle>& get_particles() const;

	unsigned int get_particles_count() const;

	void spawn_particles_sphere(int particles_num, const ParticleMaterial& material);

	void spawn_particles_cube(int particles_num, const ParticleMaterial& material);

	// The simulation always advances in steps of _timestep. If frametime il larger that _timestep, multiple iteration steps can be taken (set in main).
	void step();
};