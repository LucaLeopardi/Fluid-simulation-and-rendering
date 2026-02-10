#include <MPM/MPMSimulation.hpp>
#include <glm/gtc/random.hpp>

MPMSimulation::MPMSimulation(
	/* 
	Grid cells are always spaced by 1, and the whole simulation is scaled in rendering if needed. This way the world position -> grid index mapping can be done simply by truncating the particle local position.
	*/
	glm::uvec3 grid_size,
	const float timestep,
	const float particle_mass,
	const float boundary,
	const float boundary_elasticity,
	const glm::vec3 gravity)
	:	
	_grid_size(grid_size),
	_timestep(timestep),
	_particle_mass(particle_mass),
	_boundary(boundary),
	_boundary_elasticity(boundary_elasticity),
	gravity(gravity)
{
	// 1. Initialise MPM grid
	// Ensure that grid size at least allows for interpolation kernel size
	if (_grid_size.x < 6) _grid_size.x = 6;
	if (_grid_size.y < 6) _grid_size.y = 6;
	if (_grid_size.z < 6) _grid_size.z = 6;

	// Grid is reset at each step: no need to initialize Cells
	_grid = std::vector<Cell>(_grid_size.x * _grid_size.y * _grid_size.z);
	// Particles added and initialized in spawn_particles
}

float MPMSimulation::get_timestep() const { return _timestep; }

unsigned int MPMSimulation::get_cells_count() const { return _grid_size.x * _grid_size.y * _grid_size.z; }

const std::vector<Cell>& MPMSimulation::get_grid() const { return _grid; }

glm::uvec3 MPMSimulation::get_grid_size() const { return _grid_size; }

Cell& MPMSimulation::get_cell(unsigned int x, unsigned int y, unsigned int z) { return _grid[x * _grid_size.y * _grid_size.z + y * _grid_size.z + z]; }

Cell& MPMSimulation::get_cell(const glm::uvec3& idx) { return _grid[idx.x * _grid_size.y * _grid_size.z + idx.y * _grid_size.z + idx.z]; }

const std::vector<Particle>& MPMSimulation::get_particles() const { return _particles; }

unsigned int MPMSimulation::get_particles_count() const { return _particles.size(); }

void MPMSimulation::spawn_particles_sphere(int particles_num, const ParticleMaterial& material)
{
	float volume = ((particles_num * _particle_mass) / material.rest_density);		// V = m/d
	float radius = std::cbrtf( (3.0f / 4.0f) * (volume / 3.14159265358979323846f));	// r = cbrtf(3/4 * V/pi)
	glm::vec3 spawn_center = glm::vec3(_grid_size) / 2.0f;

	_particles.reserve(_particles.size() + particles_num);

    // 2. initialise particles
	while (particles_num > 0) {
		_particles.emplace_back(
			glm::clamp(									// Position
				glm::ballRand(radius) + spawn_center, 
				glm::vec3(1.0f), 
				glm::vec3(_grid_size) - 2.0f),	
			glm::vec3(0.0f),							// Velocity
			_particle_mass,								// Mass
			material									// Particle material
		);
		--particles_num;
	}
}

void MPMSimulation::spawn_particles_cube(int particles_num, const ParticleMaterial& material)
{
	int particles_per_side = std::cbrt(particles_num);	// Truncated, might actually spawn less particles than asked
	particles_num = particles_per_side * particles_per_side * particles_per_side;
	_particles.reserve(_particles.size() + particles_num);

	float step = 1.0f / std::cbrtf(material.rest_density / _particle_mass);
	glm::vec3 spawn_position = (glm::vec3(_grid_size) - (particles_per_side * step)) / 2.0f ;

	for (float x = 0.0f; x < particles_per_side; ++x) {
		for (float y = 0.0f; y < particles_per_side; ++y) {
			for (float z = 0.0f; z < particles_per_side; ++z) {
				_particles.emplace_back(
					glm::clamp(										// Position
						glm::vec3(x, y, z) * step + spawn_position,
						glm::vec3(1.0f),
						glm::vec3(_grid_size) - 2.0f),	
					glm::vec3(0.0f),								// Velocity
					_particle_mass,									// Mass
					material										// Particle material
				);
			}
		}
	}
}

void MPMSimulation::step()
{
	// Interpolation weights
	glm::vec3 weights[3];

    // 1. Reset scratch-pad grid completely, zero out mass and velocity for each cell
	for (Cell& cell : _grid) {
		cell.mass = 0.0f;
		cell.velocity = glm::vec3(0.0f);
	}

	// P2G 1: Scatter particle mass and momentum to the grid
    for (Particle& particle : _particles) {
		// 2.1: Calculate weights for the neighbouring cells surrounding the particle's position on the grid using an interpolation function
		glm::ivec3 cell_idx = particle.position;								// Truncated particle position index of enclosing grid Cell
		glm::vec3 cell_dist = particle.position - glm::vec3(cell_idx) - 0.5f;	// Particle distance from enclosing Cell center (Cell dimension is always 1)
		
		// Quadratic interpolation weights
		weights[0] = 0.5f * (0.5f - cell_dist) * (0.5f - cell_dist);	// 0.5 * (0.5 - d)^2
		weights[1] = 0.75f - cell_dist * cell_dist;						// 0.75 * d^2
		weights[2] = 0.5f * (0.5f + cell_dist) * (0.5f + cell_dist);	// 0.5 * (0.5 + d)^2

        // 2.3: Scatter particle's mass and momentum to the grid, using the cell's interpolation weight calculated in 2.1
        // Particle's grid neighbourhood. Out-of-bounds check is unnecessary because particles are clamped within grid domain [1, size-1]
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 3; ++y) {
				for (int z = 0; z < 3; ++z) {
					glm::ivec3 neighbour_idx (cell_idx + glm::ivec3(x, y, z) - 1);
					glm::vec3 neighbour_dist = glm::vec3(neighbour_idx) + 0.5f - particle.position;	// Particle distance to neighbouring Cell's center
					
					glm::vec3 affine_velocity = particle.velocity_gradient * neighbour_dist;

					float weight = weights[x].x * weights[y].y * weights[z].z;
					float weighted_mass = _particle_mass * weight;

					Cell& neighbour = get_cell(neighbour_idx);
					neighbour.mass += weighted_mass;
					neighbour.velocity += weighted_mass * (particle.velocity + affine_velocity);
					// ^ Actually momentum for now, not velocity
				}
			}
		}
    }

    // 2. P2G 2: transfer data from particles to grid
    for (Particle& particle : _particles) {
        // 2.1: Calculate weights for the neighbouring cells surrounding the particle's position on the grid using an interpolation function
		glm::ivec3 cell_idx = particle.position;								// Truncated particle position is closest Cell index in grid
		glm::vec3 cell_dist = particle.position - glm::vec3(cell_idx) - 0.5f;	// Particle distance from Cell center

		// Quadratic interpolation weights
		weights[0] = 0.5f * (0.5f - cell_dist) * (0.5f - cell_dist);	// 0.5 * (0.5 - d)^2
		weights[1] = 0.75f - cell_dist * cell_dist;						// 0.75 * d^2
		weights[2] = 0.5f * (0.5f + cell_dist) * (0.5f + cell_dist);	// 0.5 * (0.5 + d)^2

		// Estimate per-particle volume
		float density = 0.0f;
		// Particle's grid neighbourhood. Out-of-bounds check is unnecessary because particles are clamped within grid domain [1, size-1]
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 3; ++y) {
				for (int z = 0; z < 3; ++z) {
					glm::ivec3 neighbour_idx = cell_idx + glm::ivec3(x, y, z) - 1;
					
					float weight = weights[x].x * weights[y].y * weights[z].z;

					Cell& neighbour = get_cell(neighbour_idx);
					density += neighbour.mass * weight;
				}
			}
		}
		particle.volume = _particle_mass / density;
		
		// 2.2: Calculate quantities like e.g. stress based on constitutive equation
		// Simplified eq. of state (by nialltl)
		float pressure = particle.material.EOS_stiffness * (powf(density / particle.material.rest_density, particle.material.EOS_power) - 1.0f);
		pressure = std::max(-0.1f, pressure);	// Clamped to avoid greatly negative pressure, hacky solution to particles collapsing (by nialltl).

		// Build strain from velocity gradient
		glm::mat3 strain = particle.material.dynamic_viscosity * (particle.velocity_gradient + glm::transpose(particle.velocity_gradient));	// Strain tensor: viscosity * (velocity_gradient + transpose(velocity_gradient))

		glm::mat3 stress = glm::mat3(-pressure) + strain;	// Stress: -p * I + strain

		glm::mat3 stressContribution = -(particle.volume) * 4.0f * stress * _timestep;	// idk

        // 2.3: Scatter particle's momentum to the grid, using the cell's interpolation weight calculated in 2.1
        // Particle's grid neighbourhood. Out-of-bounds check is unnecessary because particles are clamped within grid domain [1, size-1]
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 3; ++y) {
				for (int z = 0; z < 3; ++z) {
					glm::ivec3 neighbour_idx = cell_idx + glm::ivec3(x, y, z) - 1;
					glm::vec3 neighbour_dist = glm::vec3(neighbour_idx) + 0.5f - particle.position;	// Particle distance to neighbouring Cell's center

					float weight = weights[x].x * weights[y].y * weights[z].z;

					Cell& neighbour = get_cell(neighbour_idx);
					neighbour.velocity += (stressContribution * weight) * neighbour_dist;	// Fused force + momentum update from MLS-MPM
					// ^ currently "cell.v" refers to MOMENTUM, not velocity! This gets corrected in the grid update.
				}
			}
		}
    }

    // 3. Calculate grid velocities
	for (int x = 0; x < _grid_size.x; ++x) {
		for (int y = 0; y < _grid_size.y; ++y) {
			for (int z = 0; z < _grid_size.z; ++z) {
				Cell& cell = get_cell(x, y, z);
				if (cell.mass <= 0) continue;	// Skip irrelevant cells

				// 3.1: Calculate grid velocity based on momentum found in the P2G stage
				cell.velocity /= cell.mass;				// Convert momentum to velocity
				cell.velocity += gravity * _timestep;	// Apply gravity

				// 3.2: Enforce grid boundary conditions
				if (x < 2 || x > _grid_size.x - 3) cell.velocity.x = 0.0f;
				if (y < 2 || y > _grid_size.y - 3) cell.velocity.y = 0.0f;
				if (z < 2 || z > _grid_size.z - 3) cell.velocity.z = 0.0f;
			}
		}
    }

    // 4. Grid-to-particle (G2P) data transfer.
    for (Particle& particle : _particles) {
        // 4.2: Calculate neighbouring cell weights
		glm::ivec3 cell_idx = particle.position;								// Truncated particle position is Cell grid index
		glm::vec3 cell_dist = particle.position - glm::vec3(cell_idx) - 0.5f;	// Particle distance from Cell center

		// Quadratic interpolation weights
		weights[0] = 0.5f * (0.5f - cell_dist) * (0.5f - cell_dist);	// 0.5 * (0.5 - d)^2
		weights[1] = 0.75f - cell_dist * cell_dist;						// 0.75 * d^2
		weights[2] = 0.5f * (0.5f + cell_dist) * (0.5f + cell_dist);	// 0.5 * (0.5 + d)^2

        // 4.3: Calculate our new particle velocities

		// Reset particle velocity and velocity gradient
		particle.velocity = glm::vec3(0.0f);	
		particle.velocity_gradient = glm::mat3(0.0f);
		
		// Particle's grid neighbourhood. Out-of-bounds check is unnecessary because particles are clamped within grid domain [1, size-1]
		for (int x = 0; x < 3; ++x) {
			for (int y = 0; y < 3; ++y) {
				for (int z = 0; z < 3; ++z) {
					glm::ivec3 neighbour_idx (cell_idx + glm::ivec3(x, y, z) - 1);
					glm::vec3 neighbour_dist = glm::vec3(neighbour_idx) + 0.5f - particle.position;	// Particle distance to neighbouring Cell's center

					// 4.3.1: Get this cell's weighted contribution to our particle's new velocity
					float weight = weights[x].x * weights[y].y * weights[z].z;

					Cell& neighbour = get_cell(neighbour_idx);
					glm::vec3 weightedVelocity = neighbour.velocity * weight;

					particle.velocity += weightedVelocity;
					particle.velocity_gradient += glm::mat3(		// Outer multiplication
						weightedVelocity * neighbour_dist.x,
						weightedVelocity * neighbour_dist.y,
						weightedVelocity * neighbour_dist.z
					);
				}
			}
        }
		particle.velocity_gradient *= 4.0f;	// 4 is a constant resulting from interpolation weights

        // 4.4: Advect particle positions by their velocity (explicit integration)
		// TODO: Semi-implicit integration? Other methods?
		particle.position += particle.velocity * _timestep;
		
		// Clamp particle to simulation domain [1, gridSize - 2]
		particle.position = glm::clamp(particle.position, glm::vec3(1.0f), glm::vec3(_grid_size) - 2.0f);
		
		// Additional predictive boundary conditions to soften velocities near edges.
		// Taken from nialltl's implementation, but added timestep scaling and boundary elasticity.
		// Might look weird but should improve stability.
		glm::vec3 predicted_position = particle.position + particle.velocity * _timestep;
		glm::vec3 upper_boundary = glm::vec3(_grid_size) - 1.0f - _boundary;

		if (predicted_position.x < _boundary) particle.velocity.x += (_boundary - predicted_position.x) * _boundary_elasticity;
		if (predicted_position.x > upper_boundary.x) particle.velocity.x += (upper_boundary.x - predicted_position.x) * _boundary_elasticity;
		if (predicted_position.y < _boundary) particle.velocity.y += (_boundary - predicted_position.y) * _boundary_elasticity;
		if (predicted_position.y > upper_boundary.y) particle.velocity.y += (upper_boundary.y - predicted_position.y) * _boundary_elasticity;
		if (predicted_position.z < _boundary) particle.velocity.z += (_boundary - predicted_position.z) * _boundary_elasticity;
		if (predicted_position.z > upper_boundary.z) particle.velocity.z += (upper_boundary.z - predicted_position.z) * _boundary_elasticity;

		// Check for NaN values (aka particle simulation broke)
		assert(particle.position.x == particle.position.x);
		assert(particle.position.y == particle.position.y);
		assert(particle.position.z == particle.position.z);
    }
}