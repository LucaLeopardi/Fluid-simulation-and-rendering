#include <MPM/MPMSimulation.cuh>
#include <glm/gtc/random.hpp>
#include <utils/CudaCheck.cuh>

const unsigned int MIN_GRID_SIZE = 40;
const unsigned int MAX_CELLS_NUM = 240 * 240 * 240;
const unsigned int PARTICLES_SPAWN_CUBE_SIZE = 32;	// 32 is max: used as part of kernel configuration	
const unsigned int PARTICLES_SPAWN_NUM = PARTICLES_SPAWN_CUBE_SIZE * PARTICLES_SPAWN_CUBE_SIZE * PARTICLES_SPAWN_CUBE_SIZE;
const unsigned int MAX_PARTICLES_NUM = 32 * 32 * 32 * 64;	// ~2 mln particles
const unsigned int MAX_WHITEWATER_NUM = MAX_PARTICLES_NUM / 4;	// ~524k whitewater

void MPMSimulation::_estimate_water_level()
{
	float mass = _particles_count * particles_material.mass;
	float density = particles_material.rest_density * (particles_material.EOS_stiffness / 1000.0f);	// Arbitrarily scaled by stiffness/1000, as simulation isn't actually incompressible
	_water_level = mass / (_grid_size.x * _grid_size.z * density);
}

// CUDA kernels configuration
unsigned int block_dim = 128;
unsigned int cells_grid_dim = 0;
unsigned int particles_grid_dim = 0;
unsigned int whitewater_grid_dim = 0;


// CUDA kernels declarations

__global__ void initialize_particles_sphere(
	glm::aligned_vec3* const new_particles_positions,
	glm::aligned_vec3* const new_particles_velocities,
	glm::aligned_mat3* const new_particles_velocity_gradients,
	const unsigned int end_idx,
	const glm::aligned_vec3 spawn_center,
	const float radius,
	curandState* curand_states,
	unsigned long long seed
);

__global__ void initialize_particles_cube(
	glm::aligned_vec3* const new_particles_positions,
	glm::aligned_vec3* const new_particles_velocities,
	glm::aligned_mat3* const new_particles_velocity_gradients,
	const unsigned int end_idx,
	const glm::aligned_vec3 spawn_origin,
	const float step,
	curandState* curand_states,
	unsigned long long seed
);

__global__ void grid_reset(
	glm::aligned_vec3* const cells_velocities, 
	float* const cells_masses,
	const unsigned int cells_count
);

__global__ void p2g_init(
	glm::aligned_vec3* const particles_positions,
	const unsigned int particles_count, 
	const ParticleMaterial particles_material,
	float* const cells_masses, 
	const glm::uvec3 grid_size
);

__global__ void p2g(
	glm::aligned_vec3* const particles_positions, 
	glm::aligned_vec3* const particles_velocities, 
	glm::aligned_mat3* const particles_velocity_gradients,
	const unsigned int particles_count,
	const ParticleMaterial particles_material, 
	glm::aligned_vec3* const cells_velocities,
	float* const cells_masses, 
	const glm::uvec3 grid_size, 
	const float timestep
);

__global__ void grid_update(
	glm::aligned_vec3* const cells_velocities, 
	float* const cells_masses, 
	const glm::uvec3 grid_size, 
	const float timestep, 
	const glm::aligned_vec3 gravity
);

__global__ void g2p(
	glm::aligned_vec3* const particles_positions, 
	glm::aligned_vec3* const particles_velocities, 
	glm::aligned_mat3* const particles_velocity_gradients, 
	curandState* curand_states,
	const unsigned int particles_count, 
	glm::aligned_vec3* const cells_velocities,
	const glm::uvec3 grid_size,
	glm::aligned_vec3* const whitewater_positions,
	glm::aligned_vec3* const whitewater_velocities,
	float* const whitewater_lifetimes,
	const unsigned int new_whitewater_start_idx,
	const unsigned int new_whitewater_max_idx,
	unsigned int* const spawned_whitewater_counter,
	const float whitewater_chance_min,
	const float whitewater_chance_max,
	const unsigned int whitewater_spawn_num,
	const float timestep, 
	const float boundary, 
	const float boundary_elasticity
);

__global__ void advect_whitewater(
	glm::aligned_vec3* const whitewater_positions,
	glm::aligned_vec3* const whitewater_velocities,
	GLubyte* const whitewater_types,
	float* const whitewater_lifetimes,
	const unsigned int whitewater_start_idx,
	const unsigned int whitewater_end_idx,
	const unsigned int moved_whitewater_start_idx,
	const unsigned int moved_whitewater_max_idx,
	unsigned int* const moved_whitewater_counter,
	const glm::aligned_vec3* const cells_velocities, 
	const float* const cells_masses, 
	const glm::uvec3 grid_size,
	const ParticleMaterial particles_material, 
	const float timestep,
	const glm::aligned_vec3 gravity,
	const float boundary, 
	const float boundary_elasticity
);


MPMSimulation::MPMSimulation(
	/* 
	Grid cells are always spaced by 1, and the whole simulation is scaled in rendering if needed. This way the world position -> grid index mapping can be done simply by truncating the particle local position.
	*/
	glm::uvec3 grid_size,
	const ParticleMaterial& particles_material,
	const float timestep,
	const float boundary,
	const float boundary_elasticity,
	const glm::aligned_vec3 gravity)
	:
	particles_material(particles_material),
	_particles_count(0),
	_whitewater_count(0),
	_whitewater_start_idx(MAX_WHITEWATER_NUM),	// Will be flipped to 0 at first step()
	whitewater_chance_min(0.5f),
	whitewater_chance_max(1.0f),
	whitewater_spawn_num(10),
	_timestep(timestep),
	boundary(boundary),
	boundary_elasticity(boundary_elasticity),
	gravity(gravity)
{
	// Initialize MPM grid cells data structures
	set_grid_size(grid_size);	// Ensure that MPM grid size at least allows for interpolation kernel size
	spawn_position = floor(glm::vec3(grid_size) / 2.0f);
	glGenVertexArrays(1, &_cells_VAO);
	glBindVertexArray(_cells_VAO);

	glGenBuffers(1, &_cells_velocities_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _cells_velocities_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CELLS_NUM * sizeof(glm::aligned_vec3), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::aligned_vec3), (void*) 0);
	glEnableVertexAttribArray(0);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_cells_velocities, _cells_velocities_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	glGenBuffers(1, &_cells_masses_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _cells_masses_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CELLS_NUM * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(1);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_cells_masses, _cells_masses_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );
	
	// Initialize particles data structures
	glGenVertexArrays(1, &_particles_VAO);
	glBindVertexArray(_particles_VAO);

	// Used in instanced quad rendering, not in simulation
	glGenBuffers(1, &_quad_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _quad_VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), &_quad_vertices[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &_particles_positions_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _particles_positions_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_NUM * sizeof(glm::aligned_vec3), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::aligned_vec3), (void*) 0);
	glVertexAttribDivisor(1, 1);	// For instanced drawing
	glEnableVertexAttribArray(1);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_particles_positions, _particles_positions_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	glGenBuffers(1, &_particles_velocities_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _particles_velocities_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES_NUM * sizeof(glm::aligned_vec3), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::aligned_vec3), (void*) 0);
	glVertexAttribDivisor(2, 1);	// For instanced drawing
	glEnableVertexAttribArray(2);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_particles_velocities, _particles_velocities_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	CUDA_CHECK( cudaMalloc(&_d_particles_velocity_gradients, MAX_PARTICLES_NUM * sizeof(glm::aligned_mat3)) );
	CUDA_CHECK( cudaGetLastError() );
	
	CUDA_CHECK( cudaMalloc(&_d_curand_states, MAX_PARTICLES_NUM * sizeof(curandState)) );
	CUDA_CHECK( cudaGetLastError() );

	// Initialize whitewater data structures
	CUDA_CHECK( cudaMalloc(&_d_new_whitewater_counter, sizeof(unsigned int)) );
	CUDA_CHECK( cudaGetLastError() );
	// Note: whitewater buffers are actually DOUBLE MAX_WHITEWATER_NUM, as we ping-pong between the two halves of the buffers each frame.
	glGenVertexArrays(1, &_whitewater_VAO);
	glBindVertexArray(_whitewater_VAO);

	// Used in instanced quad rendering, not in simulation. Already initialized for particles, only needs binding.
	glBindBuffer(GL_ARRAY_BUFFER, _quad_VBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &_whitewater_positions_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _whitewater_positions_VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * MAX_WHITEWATER_NUM * sizeof(glm::aligned_vec3), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::aligned_vec3), (void*) 0);
	glVertexAttribDivisor(1, 1);	// For instanced drawing
	glEnableVertexAttribArray(1);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_whitewater_positions, _whitewater_positions_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	CUDA_CHECK( cudaMalloc(&_d_whitewater_velocities, 2 * MAX_WHITEWATER_NUM * sizeof(glm::aligned_vec3)) );
	CUDA_CHECK( cudaGetLastError() );

	glGenBuffers(1, &_whitewater_types_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _whitewater_types_VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * MAX_WHITEWATER_NUM * sizeof(GLubyte), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(GLubyte), (void*) 0);
	glVertexAttribDivisor(2, 1);	// For instanced drawing
	glEnableVertexAttribArray(2);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_whitewater_types, _whitewater_types_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	glGenBuffers(1, &_whitewater_lifetimes_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _whitewater_lifetimes_VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * MAX_WHITEWATER_NUM * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*) 0);
	glVertexAttribDivisor(3, 1);	// For instanced drawing
	glEnableVertexAttribArray(3);
	CUDA_CHECK( cudaGraphicsGLRegisterBuffer(&_whitewater_lifetimes, _whitewater_lifetimes_VBO, cudaGraphicsRegisterFlagsNone) );
	CUDA_CHECK( cudaGetLastError() );

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void MPMSimulation::cleanup()
{
	CUDA_CHECK( cudaFree(_d_curand_states) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_cells_velocities) )
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_cells_masses) )
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_particles_positions) )
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_particles_velocities) )
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_whitewater_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_whitewater_types) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnregisterResource(_whitewater_lifetimes) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaFree(_d_new_whitewater_counter) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceReset() );

	glDeleteBuffers(1, &_cells_velocities_VBO);
	glDeleteBuffers(1, &_cells_masses_VBO);
	glDeleteBuffers(1, &_particles_positions_VBO);
	glDeleteBuffers(1, &_particles_velocities_VBO);
	glDeleteBuffers(1, &_whitewater_positions_VBO);
	glDeleteBuffers(1, &_whitewater_types_VBO);
	glDeleteBuffers(1, &_whitewater_lifetimes_VBO);
}


GLuint MPMSimulation::get_cells_VAO() const { return _cells_VAO; }

GLuint MPMSimulation::get_particles_VAO() const { return _particles_VAO; }

GLuint MPMSimulation::get_whitewater_VAO() const { return _whitewater_VAO; }

float MPMSimulation::get_timestep() const { return _timestep; }

glm::uvec3 MPMSimulation::get_grid_size() const { return _grid_size; }

void MPMSimulation::set_grid_size(glm::uvec3 size)
{
	_grid_size = size;
	// Enforce minimum grid size
	if (_grid_size.x < MIN_GRID_SIZE) _grid_size.x = MIN_GRID_SIZE;
	if (_grid_size.y < MIN_GRID_SIZE) _grid_size.y = MIN_GRID_SIZE;
	if (_grid_size.z < MIN_GRID_SIZE) _grid_size.z = MIN_GRID_SIZE;

	// Update cells kernels configuration
	cells_grid_dim = (get_cells_count() + block_dim - 1) / block_dim;

	if (spawn_position.x > _grid_size.x - 20.0f) spawn_position.x = _grid_size.x - 20.0f;
	if (spawn_position.y > _grid_size.y - 20.0f) spawn_position.y = _grid_size.y - 20.0f;
	if (spawn_position.z > _grid_size.z - 20.0f) spawn_position.z = _grid_size.z - 20.0f;

	// Update water level estimate
	_estimate_water_level();
}

float MPMSimulation::get_estimated_water_level() const { return _water_level; }

unsigned int MPMSimulation::get_cells_count() const { return _grid_size.x * _grid_size.y * _grid_size.z; }

unsigned int MPMSimulation::get_particles_count() const { return _particles_count; }

unsigned int MPMSimulation::get_particles_max() const { return MAX_PARTICLES_NUM; }

unsigned int MPMSimulation::get_whitewater_start_idx() const { return _whitewater_start_idx; }

unsigned int MPMSimulation::get_whitewater_count() const { return _whitewater_count; }

unsigned int MPMSimulation::get_whitewater_max() const { return MAX_WHITEWATER_NUM; }

bool MPMSimulation::can_spawn_particles() const { return _particles_count + PARTICLES_SPAWN_NUM <= MAX_PARTICLES_NUM; }


void MPMSimulation::reset_simulation()
{
	// No need to delete particles: reset particle counter, and old data in the particles buffers will be overwritten when new ones are initialized.
	_particles_count = 0;
	_whitewater_count = 0;
	_estimate_water_level();

	// Reset grid cells. These DO need to be reset, or they won't be until new particles are spawned (as the simulation doesn't run if there are zero particles).
	glm::aligned_vec3* d_cells_velocities;
	float* d_cells_masses;
	
	CUDA_CHECK( cudaGraphicsMapResources(1, &_cells_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_cells_velocities, NULL, _cells_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	
	CUDA_CHECK( cudaGraphicsMapResources(1, &_cells_masses) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_cells_masses, NULL, _cells_masses) );
	CUDA_CHECK( cudaGetLastError() );

	grid_reset<<<cells_grid_dim, block_dim>>>(
		d_cells_velocities, 
		d_cells_masses,
		get_cells_count());
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_cells_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_cells_masses) );
	CUDA_CHECK( cudaGetLastError() );
}


void MPMSimulation::spawn_particles_sphere()
{
	if (!can_spawn_particles()) return;

	const unsigned int new_particles_count = _particles_count + PARTICLES_SPAWN_NUM;
	// Find sphere volume to spawn particles in rest state
	float volume = ((PARTICLES_SPAWN_NUM * particles_material.mass) / particles_material.rest_density);		// V = m/d
	float radius = std::cbrtf( (3.0f / 4.0f) * (volume / 3.14159265358979323846f));	// r = cbrtf(3/4 * V/pi)

	// Ensure spawn volume is within bounds. If not, shrink. Particles won't spawn in rest state anymore.
	if (radius > (_grid_size.x / 2.0f) - 1.0f) radius = (_grid_size.x / 2.0f) - 1.0f;
	if (radius > (_grid_size.y / 2.0f) - 1.0f) radius = (_grid_size.y / 2.0f) - 1.0f;
	if (radius > (_grid_size.z / 2.0f) - 1.0f) radius = (_grid_size.z / 2.0f) - 1.0f;

	// Map CUDA resources
	CUDA_CHECK( cudaGraphicsMapResources(1, &_particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsMapResources(1, &_particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	// Get pointers
	glm::aligned_vec3* d_particles_positions;
	glm::aligned_vec3* d_particles_velocities;
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_positions, NULL, _particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_velocities, NULL, _particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );

	// Initialise particles with kernel
	// TODO: Change configurations if spawning a different number or shape of particles
	initialize_particles_sphere<<<PARTICLES_SPAWN_CUBE_SIZE, dim3(PARTICLES_SPAWN_CUBE_SIZE, PARTICLES_SPAWN_CUBE_SIZE, 1)>>>(
		&d_particles_positions[_particles_count],			// Pointer to first element of array to initialize
		&d_particles_velocities[_particles_count],			// Pointer to first element of array to initialize
		&_d_particles_velocity_gradients[_particles_count],	// Pointer to first element of array to initialize
		new_particles_count,								// Index to stop at
		spawn_position,
		radius,
		&_d_curand_states[_particles_count],				// Pointer to first element of array to initialize
		0);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// Unmap
	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );

	// Update particles count;
	_particles_count = new_particles_count;
	particles_grid_dim = (_particles_count + block_dim - 1) / block_dim;
	_estimate_water_level();
}


void MPMSimulation::spawn_particles_cube()
{
	if (!can_spawn_particles()) return;

	const unsigned int new_particles_count = _particles_count + PARTICLES_SPAWN_NUM;

	// Ensure spawn volume is within bounds
	float step = 1.0f / std::cbrtf(particles_material.rest_density / particles_material.mass);
	if (step * PARTICLES_SPAWN_CUBE_SIZE > _grid_size.x - 2.0f) step = (_grid_size.x - 2.0f) / PARTICLES_SPAWN_CUBE_SIZE;
	if (step * PARTICLES_SPAWN_CUBE_SIZE > _grid_size.y - 2.0f) step = (_grid_size.y - 2.0f) / PARTICLES_SPAWN_CUBE_SIZE;
	if (step * PARTICLES_SPAWN_CUBE_SIZE > _grid_size.z - 2.0f) step = (_grid_size.z - 2.0f) / PARTICLES_SPAWN_CUBE_SIZE;
	glm::aligned_vec3 spawn_origin = spawn_position - floor(PARTICLES_SPAWN_CUBE_SIZE * step / 2.0f);

	// Map CUDA resources
	CUDA_CHECK( cudaGraphicsMapResources(1, &_particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsMapResources(1, &_particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	// Get pointers
	glm::aligned_vec3* d_particles_positions;
	glm::aligned_vec3* d_particles_velocities;
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_positions, NULL, _particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_velocities, NULL, _particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );

	// Initialise particles with kernel
	// TODO: Change configurations if spawning a different number or shape of particles
	initialize_particles_cube<<<PARTICLES_SPAWN_CUBE_SIZE, dim3(PARTICLES_SPAWN_CUBE_SIZE, PARTICLES_SPAWN_CUBE_SIZE, 1)>>>(
		&d_particles_positions[_particles_count],			// Pointer to first element of array to initialize
		&d_particles_velocities[_particles_count],			// Pointer to first element of array to initialize
		&_d_particles_velocity_gradients[_particles_count],	// Pointer to first element of array to initialize
		new_particles_count,								// Index to stop at
		spawn_origin,
		step,
		&_d_curand_states[_particles_count],				// Pointer to first element of array to initialize
		0);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// Unmap
	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsUnmapResources(1, &_particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );

	// Update particles count;
	_particles_count = new_particles_count;
	particles_grid_dim = (_particles_count + block_dim - 1) / block_dim;
	_estimate_water_level();
}


void MPMSimulation::step()
{
	if (_particles_count == 0) return;

	// Map OpenGL resources to CUDA
	glm::aligned_vec3 *d_particles_positions, *d_particles_velocities, *d_cells_velocities, *d_whitewater_positions;
	GLubyte *d_whitewater_types;
	float *d_cells_masses, *d_whitewater_lifetimes;

	cudaGraphicsResource* cuda_resources[] = {
		_cells_velocities,
		_cells_masses,
		_particles_positions,
		_particles_velocities,
		_whitewater_positions,
		_whitewater_types,
		_whitewater_lifetimes,
	};
	CUDA_CHECK( cudaGraphicsMapResources(sizeof(cuda_resources) / sizeof(*cuda_resources), cuda_resources) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_cells_velocities, NULL, _cells_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_cells_masses, NULL, _cells_masses) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_positions, NULL, _particles_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_particles_velocities, NULL, _particles_velocities) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_whitewater_positions, NULL, _whitewater_positions) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_whitewater_types, NULL, _whitewater_types) );
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaGraphicsResourceGetMappedPointer((void**) &d_whitewater_lifetimes, NULL, _whitewater_lifetimes) );
	CUDA_CHECK( cudaGetLastError() );
	
	// 1. Reset scratch-pad grid completely, zero out mass and velocity for each cell
	grid_reset<<<cells_grid_dim, block_dim>>>(
		d_cells_velocities,
		d_cells_masses,
		get_cells_count());
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// P2G 1 (init): Scatter particle mass to the grid
	p2g_init<<<particles_grid_dim, block_dim>>>(
		d_particles_positions,
		_particles_count, 
		particles_material, 
		d_cells_masses, 
		_grid_size);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// 2. P2G 2: transfer data from particles to our grid
	p2g<<<particles_grid_dim, block_dim>>>(
		d_particles_positions, 
		d_particles_velocities, 
		_d_particles_velocity_gradients, 
		_particles_count, 
		particles_material, 
		d_cells_velocities,
		d_cells_masses,
		_grid_size, 
		_timestep);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	 // 3. Calculate grid velocities
	grid_update<<<cells_grid_dim, block_dim>>>(
		d_cells_velocities, 
		d_cells_masses, 
		_grid_size, 
		_timestep, 
		gravity);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// The two halves of the whitewater buffer are ping-ponged: spawn new ones in inactive half, and advect ones in active half (then move them to inactive half)
	unsigned int moved_whitewater_start_idx = _whitewater_start_idx == 0 ? MAX_WHITEWATER_NUM : 0;
	CUDA_CHECK( cudaMemset(_d_new_whitewater_counter, 0, sizeof(unsigned int)) );
	CUDA_CHECK( cudaGetLastError() );

	// 4. Grid-to-particle (G2P). Transfer cells velocity data to particles and advect. Spawn new whitewater.
	g2p<<<particles_grid_dim, block_dim>>>(
		d_particles_positions, 
		d_particles_velocities, 
		_d_particles_velocity_gradients, 
		_d_curand_states,
		_particles_count, 
		d_cells_velocities,
		_grid_size,
		d_whitewater_positions,
		_d_whitewater_velocities,
		d_whitewater_lifetimes,
		moved_whitewater_start_idx,
		moved_whitewater_start_idx + MAX_WHITEWATER_NUM,
		_d_new_whitewater_counter,
		whitewater_chance_min,
		whitewater_chance_max,
		whitewater_spawn_num,
		_timestep,
		boundary,
		boundary_elasticity);
	CUDA_CHECK( cudaGetLastError() );
	CUDA_CHECK( cudaDeviceSynchronize() );

	// Advect whitewater and move surviving ones to other buffer
	if (_whitewater_count > 0) {
		advect_whitewater<<<whitewater_grid_dim, block_dim>>>(
			d_whitewater_positions,
			_d_whitewater_velocities,
			d_whitewater_types,
			d_whitewater_lifetimes,
			_whitewater_start_idx,
			_whitewater_start_idx + _whitewater_count,
			moved_whitewater_start_idx,
			moved_whitewater_start_idx + MAX_WHITEWATER_NUM,
			_d_new_whitewater_counter,
			d_cells_velocities,
			d_cells_masses,
			_grid_size,
			particles_material,
			_timestep,
			gravity,
			boundary,
			boundary_elasticity
		);
		CUDA_CHECK( cudaGetLastError() );
		CUDA_CHECK( cudaDeviceSynchronize() );
	}
	// Update whitewater count and kernel configuration
	CUDA_CHECK( cudaMemcpy(&_whitewater_count, _d_new_whitewater_counter, sizeof(unsigned int), cudaMemcpyDeviceToHost) );
	CUDA_CHECK( cudaGetLastError() );
	if (_whitewater_count > MAX_WHITEWATER_NUM) _whitewater_count = MAX_WHITEWATER_NUM;	// Kernels don't spawn whitewater exceeding MAX_WHITEWATER_NUM, but the counter is increased without limit: we need to adjust it.
	whitewater_grid_dim = (_whitewater_count + block_dim - 1) / block_dim;
	// Flip active and inactive whitewater buffer half fir rendering
	_whitewater_start_idx = moved_whitewater_start_idx;
	
	// Unmap CUDA resources
	CUDA_CHECK( cudaGraphicsUnmapResources(sizeof(cuda_resources) / sizeof(*cuda_resources), cuda_resources) );
	CUDA_CHECK( cudaGetLastError() );
}


// CUDA kernels


__global__ void initialize_particles_sphere(
	glm::aligned_vec3* const new_particles_positions,
	glm::aligned_vec3* const new_particles_velocities,
	glm::aligned_mat3* const new_particles_velocity_gradients,
	const unsigned int end_idx,
	const glm::aligned_vec3 spawn_center,
	const float radius,
	curandState* curand_states,
	unsigned long long seed)
{
	unsigned int particle_idx = threadIdx.x + threadIdx.y * blockDim.x + blockIdx.x * (blockDim.x * blockDim.y);
	if (particle_idx >= end_idx) return;
	
	new_particles_velocities[particle_idx] = glm::aligned_vec3(0.0f);
	new_particles_velocity_gradients[particle_idx] = glm::aligned_mat3(0.0f);

	curandState* state = &curand_states[particle_idx];
	curand_init(seed, particle_idx, 0, state);

	float x, y, z;
	do {
		x = radius * (curand_uniform(state) * 2.0f - 1.0f);
		y = radius * (curand_uniform(state) * 2.0f - 1.0f);
		z = radius * (curand_uniform(state) * 2.0f - 1.0f);
	} while (x * x + y * y + z * z  > radius * radius);

	new_particles_positions[particle_idx] = spawn_center + glm::aligned_vec3(x, y, z);
}


__global__ void initialize_particles_cube(
	glm::aligned_vec3* const new_particles_positions,
	glm::aligned_vec3* const new_particles_velocities,
	glm::aligned_mat3* const new_particles_velocity_gradients,
	const unsigned int end_idx,
	const glm::aligned_vec3 spawn_origin,
	const float step,
	curandState* curand_states,
	unsigned long long seed)
{
	unsigned int particle_idx = threadIdx.x + threadIdx.y * blockDim.x + blockIdx.x * (blockDim.x * blockDim.y);
	if (particle_idx >= end_idx) return;
	
	new_particles_positions[particle_idx] = spawn_origin + glm::aligned_vec3(threadIdx.x * step, threadIdx.y * step, blockIdx.x * step);
	new_particles_velocities[particle_idx] = glm::aligned_vec3(0.0f);
	new_particles_velocity_gradients[particle_idx] = glm::aligned_mat3(0.0f);

	curand_init(seed, particle_idx, 0, &curand_states[particle_idx]);
}


__global__ void grid_reset(
	glm::aligned_vec3* const cells_velocities, 
	float* const cells_masses,
	const unsigned int cells_count)
{
	unsigned int cell_idx = threadIdx.x + blockIdx.x * blockDim.x;
	if (cell_idx >= cells_count) return;
	
	cells_velocities[cell_idx] = glm::aligned_vec3(0.0f);
	cells_masses[cell_idx] = 0.0f;
}


__global__ void p2g_init(
	glm::aligned_vec3* const particles_positions,
	const unsigned int particles_count, 
	const ParticleMaterial particles_material,
	float* const cells_masses, 
	const glm::uvec3 grid_size)
{
	unsigned int particle_idx = threadIdx.x + blockIdx.x * blockDim.x;
	if (particle_idx >= particles_count) return;
	const glm::aligned_vec3 particle_position = particles_positions[particle_idx];

	// Calculate weights for the neighbouring cells surrounding the particle's position on the grid using an interpolation function
	const glm::ivec3 cell_coords = particle_position;								// Truncated Particle position is the grid coords of the enclosing Cell
	const glm::aligned_vec3 cell_dist = particle_position - glm::aligned_vec3(cell_coords) -0.5f;	// Particle distance to enclosing Cell's center (because Cell dimension is always 1)

	// Quadratic interpolation weights
	const glm::aligned_vec3 weights[3] = {
		0.5f * (0.5f - cell_dist) * (0.5f - cell_dist),		// 0.5 * (0.5 - d)^2
		0.75f - cell_dist * cell_dist,						// 0.75 - d^2
		0.5f * (0.5f + cell_dist) * (0.5f + cell_dist)		// 0.5 * (0.5 + d)^2
	};

	unsigned int grid_size_yz = grid_size.y * grid_size.z;
	// Scatter Particle's mass to the grid, using the cell's interpolation weights
	// Particle's grid neighbourhood
	#pragma unroll
	for (int x = 0; x < 3; ++x) {
		#pragma unroll
		for (int y = 0; y < 3; ++y) {
			#pragma unroll
			for (int z = 0; z < 3; ++z) {
				glm::ivec3 n_cell_coords (cell_coords + glm::ivec3(x, y, z) - 1);
				float weight = weights[x].x * weights[y].y * weights[z].z;
				atomicAdd(&cells_masses[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z], weight * particles_material.mass);
			}
		}
	}
}


__global__ void p2g(
	glm::aligned_vec3* const particles_positions, 
	glm::aligned_vec3* const particles_velocities, 
	glm::aligned_mat3* const particles_velocity_gradients,
	const unsigned int particles_count, 
	const ParticleMaterial particles_material, 
	glm::aligned_vec3* const cells_velocities,
	float* const cells_masses,
	const glm::uvec3 grid_size, 
	const float timestep)
{
	unsigned int particle_idx = threadIdx.x + blockIdx.x * blockDim.x;
	if (particle_idx >= particles_count) return;

	const glm::aligned_vec3 particle_position = particles_positions[particle_idx];
	const glm::aligned_vec3 particle_velocity = particles_velocities[particle_idx];
	const glm::aligned_mat3 particle_velocity_gradient = particles_velocity_gradients[particle_idx];

	const glm::ivec3 cell_coords = particle_position;									// Truncated Particle position is the index of the enclosing grid Cell
	const glm::aligned_vec3 cell_dist = particle_position - glm::aligned_vec3(cell_coords) - 0.5f;	// Particle distance to enclosing Cell's center (because Cell dimension is always 1)

	// Quadratic interpolation weights for the neighbouring cells surrounding the particle
	const glm::aligned_vec3 weights[3] = {
		0.5f * (0.5f - cell_dist) * (0.5f - cell_dist),		// 0.5 * (0.5 - d)^2
		0.75f - cell_dist * cell_dist,						// 0.75 - d^2
		0.5f * (0.5f + cell_dist) * (0.5f + cell_dist)		// 0.5 * (0.5 + d)^2
	};

	// Estimate per-particle density
	float density = 0.0f;
	unsigned int grid_size_yz = grid_size.y * grid_size.z;
	// Particle's grid neighbourhood
	#pragma unroll
	for (int x = 0; x < 3; ++x) {
		#pragma unroll
		for (int y = 0; y < 3; ++y) {
			#pragma unroll
			for (int z = 0; z < 3; ++z) {
				glm::ivec3 n_cell_coords = cell_coords + glm::ivec3(x, y, z) - 1;
				float weight = weights[x].x * weights[y].y * weights[z].z;
				density += weight * cells_masses[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z];
			}
		}
	}
	// 2.2: Calculate quantities like e.g. stress based on constitutive equation
	// Simplified eq. of state (by nialltl)
	// p = stiffness * ((density / rest_density) ^ pow) - 1)
	float pressure = particles_material.EOS_stiffness * (powf(density / particles_material.rest_density, particles_material.EOS_power) - 1.0f);
	pressure = pressure < particles_material.max_negative_pressure ? particles_material.max_negative_pressure : pressure;	// Clamped to avoid greatly negative pressure, hacky solution to particles collapsing (by nialltl).

	// Strain rate tensor = viscosity * (velocity_gradient + transpose(velocity_gradient))
	const glm::aligned_mat3 strain = particles_material.dynamic_viscosity * (particle_velocity_gradient + glm::transpose(particle_velocity_gradient));	
	// Stress = -p * I + strain
	const glm::aligned_mat3 stress = glm::aligned_mat3(-pressure) + strain;
	// Stress_contribution = -V * 4 * stress * dt
	const glm::aligned_mat3 stress_contribution = -(particles_material.mass / density) * 4.0f * stress * timestep;

	// 2.3: Scatter particle's momentum to the grid, using the cell's interpolation weight calculated in 2.1
	// Particle's grid neighbourhood. 
	#pragma unroll
	for (int x = 0; x < 3; ++x) {
		#pragma unroll
		for (int y = 0; y < 3; ++y) {
			#pragma unroll
			for (int z = 0; z < 3; ++z) {
				glm::ivec3 n_cell_coords = cell_coords + glm::ivec3(x, y, z) - 1;
				glm::aligned_vec3 n_cell_dist = glm::aligned_vec3(n_cell_coords) + 0.5f - particle_position;	// Particle distance to neighbouring Cell's center

				float weight = weights[x].x * weights[y].y * weights[z].z;

				// Fused force + momentum update from MLS-MPM
				glm::aligned_vec3 affine_velocity = n_cell_dist * particle_velocity_gradient;
				glm::aligned_vec3 n_cell_momentum = weight * particles_material.mass * (particle_velocity + affine_velocity);
				n_cell_momentum += stress_contribution * weight * n_cell_dist;

				glm::aligned_vec3& n_cell_velocity = cells_velocities[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z];
				atomicAdd(&n_cell_velocity.x, n_cell_momentum.x);
				atomicAdd(&n_cell_velocity.y, n_cell_momentum.y);
				atomicAdd(&n_cell_velocity.z, n_cell_momentum.z);
			}
		}
	}
}


__global__ void grid_update(
	glm::aligned_vec3* const cells_velocities, 
	float* const cells_masses, 
	const glm::uvec3 grid_size, 
	const float timestep, 
	const glm::aligned_vec3 gravity)
{
	unsigned int cell_idx = threadIdx.x + blockIdx.x * blockDim.x;
	if (cell_idx >= grid_size.x * grid_size.y * grid_size.z) return;
	if (cells_masses[cell_idx] <= 0) return;	// Skip irrelevant cells

	// 3.1: Calculate grid velocity based on momentum found in the P2G stage
	glm::aligned_vec3& cell_velocity = cells_velocities[cell_idx];
	cell_velocity /= cells_masses[cell_idx];	// Convert momentum to velocity
	cell_velocity += gravity * timestep;		// Apply gravity

	// 3.2: Enforce grid boundary conditions
	unsigned int cell_x = cell_idx / (grid_size.y * grid_size.z);
	unsigned int cell_y = (cell_idx % (grid_size.y * grid_size.z)) / grid_size.z;
	unsigned int cell_z = (cell_idx % (grid_size.y * grid_size.z)) % grid_size.z; 
	if (cell_x < 2 || cell_x > grid_size.x - 3) cell_velocity.x = 0.0f;
	if (cell_y < 2 || cell_y > grid_size.y - 3) cell_velocity.y = 0.0f;
	if (cell_z < 2 || cell_z > grid_size.z - 3) cell_velocity.z = 0.0f;
}


__global__ void g2p(
	glm::aligned_vec3* const particles_positions, 
	glm::aligned_vec3* const particles_velocities, 
	glm::aligned_mat3* const particles_velocity_gradients, 
	curandState* curand_states,
	const unsigned int particles_count, 
	glm::aligned_vec3* const cells_velocities,
	const glm::uvec3 grid_size,
	glm::aligned_vec3* const whitewater_positions,
	glm::aligned_vec3* const whitewater_velocities,
	float* const whitewater_lifetimes,
	const unsigned int new_whitewater_start_idx,
	const unsigned int new_whitewater_max_idx,
	unsigned int* const spawned_whitewater_counter,
	const float whitewater_chance_min,
	const float whitewater_chance_max,
	const unsigned int whitewater_spawn_num,
	const float timestep, 
	const float boundary, 
	const float boundary_elasticity)
{
	unsigned int particle_idx = threadIdx.x + blockIdx.x * blockDim.x;
	if (particle_idx >= particles_count) return;
	
	glm::aligned_vec3 particle_position = particles_positions[particle_idx];
	const glm::aligned_vec3 old_particle_velocity = particles_velocities[particle_idx];

	// Calculate weights for the neighbouring cells surrounding the particle's position on the grid using an interpolation function
	const glm::ivec3 cell_coords = particle_position;								// Truncated Particle position is the grid coords of the enclosing Cell
	const glm::aligned_vec3 cell_dist = particle_position - glm::aligned_vec3(cell_coords) - 0.5f;	// Particle distance to enclosing Cell's center (because Cell dimension is always 1)

	// Quadratic interpolation weights
	const glm::aligned_vec3 weights[3] = {
		0.5f * (0.5f - cell_dist) * (0.5f - cell_dist),		// 0.5 * (0.5 - d)^2
		0.75f - cell_dist * cell_dist,						// 0.75 - d^2
		0.5f * (0.5f + cell_dist) * (0.5f + cell_dist)		// 0.5 * (0.5 + d)^2
	};

	// 4.3: Calculate new particle velocities
	// Reset particle velocity and velocity gradient
	glm::aligned_vec3 new_particle_velocity (0.0f);	
	glm::aligned_mat3 new_particle_velocity_gradient (0.0f);
	float turbulence = 0.0f;
	
	unsigned int grid_size_yz = grid_size.y * grid_size.z;
	// Particle's grid neighbourhood. 
	#pragma unroll
	for (int x = 0; x < 3; ++x) {
		#pragma unroll
		for (int y = 0; y < 3; ++y) {
			#pragma unroll
			for (int z = 0; z < 3; ++z) {
				glm::ivec3 n_cell_coords (cell_coords + glm::ivec3(x, y, z) - 1);
				glm::aligned_vec3 n_cell_dist = glm::aligned_vec3(n_cell_coords) + 0.5f - particle_position;	// Particle distance to neighbouring Cell's center

				// 4.3.1: Get this cell's weighted contribution to our particle's new velocity
				float weight = weights[x].x * weights[y].y * weights[z].z;
				glm::aligned_vec3 n_cell_velocity = cells_velocities[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z];
				glm::aligned_vec3 weighted_velocity = weight * n_cell_velocity;

				new_particle_velocity += weighted_velocity;
				// Outer multiplication
				new_particle_velocity_gradient += 4.0f * glm::aligned_mat3(		// 4 is a constant resulting from interpolation weights
					weighted_velocity * n_cell_dist.x,
					weighted_velocity * n_cell_dist.y,
					weighted_velocity * n_cell_dist.z
				);
				// Calculate turbulence for whitewater spawn
				glm::aligned_vec3 relative_vel = old_particle_velocity - n_cell_velocity;
				glm::aligned_vec3 relative_vel_direction = glm::normalize(relative_vel);
				float relative_vel_magnitude = glm::length(relative_vel);
				// Measures the amount of trapped air: 2.0 for particles colliding, 0.0 for particles moving away. From "Unified Spray, Foam, and whitewater for Particle-Based Fluids" (Ihmsen et al.)
				turbulence += weight * relative_vel_magnitude * (1.0f - glm::dot(relative_vel_direction, glm::normalize(n_cell_dist)));
			}
		}
	}

	// 4.4: Advect particle positions by their velocity (explicit integration)
	glm::aligned_vec3 dx = new_particle_velocity * timestep;
	particle_position += dx;
	// Clamp particle to simulation domain [1, gridSize - 2]
	particle_position = glm::clamp(particle_position, glm::aligned_vec3(1.0f), glm::aligned_vec3(grid_size) - 2.0f);
	
	// Additional predictive boundary conditions to soften velocities near edges.
	// Taken from nialltl's implementation, but added timestep scaling and boundary elasticity.
	// Might look unnatural but should improve stability.
	if (boundary > 0.0f && boundary_elasticity > 0.0f) {
		const glm::aligned_vec3 predicted_position = particle_position + dx;
		const glm::aligned_vec3 upper_boundary = glm::aligned_vec3(grid_size) - 1.0f - boundary;

		if (predicted_position.x < boundary) new_particle_velocity.x += (boundary - predicted_position.x) * boundary_elasticity;
		if (predicted_position.y < boundary) new_particle_velocity.y += (boundary - predicted_position.y) * boundary_elasticity;
		if (predicted_position.z < boundary) new_particle_velocity.z += (boundary - predicted_position.z) * boundary_elasticity;
		if (predicted_position.x > upper_boundary.x) new_particle_velocity.x += (upper_boundary.x - predicted_position.x) * boundary_elasticity;
		if (predicted_position.y > upper_boundary.y) new_particle_velocity.y += (upper_boundary.y - predicted_position.y) * boundary_elasticity;
		if (predicted_position.z > upper_boundary.z) new_particle_velocity.z += (upper_boundary.z - predicted_position.z) * boundary_elasticity;
	}
	particles_positions[particle_idx] = particle_position;
	particles_velocities[particle_idx] = new_particle_velocity;
	particles_velocity_gradients[particle_idx] = new_particle_velocity_gradient;

	// Check for NaN values (aka if particle simulation broke)
	#ifdef ENABLE_ASSERTS
		assert(particle_position.x == particle_position.x);
		assert(particle_position.y == particle_position.y);
		assert(particle_position.z == particle_position.z);
	#endif

	// Spawn whitewater, with increased likeliness and amount depending on kinetic energy and turbulence
	float kinetic_energy = dot(new_particle_velocity, new_particle_velocity); 
	float kinetic_energy_factor = (min(kinetic_energy, whitewater_chance_max) - min(kinetic_energy, whitewater_chance_min)) / (whitewater_chance_max - whitewater_chance_min);	// Normalized on min and max
	float trapped_air_factor = (min(turbulence, whitewater_chance_max) - min(turbulence, whitewater_chance_min)) / (whitewater_chance_max - whitewater_chance_min);				// Normalized on min and max
	float spawn_chance = kinetic_energy_factor * trapped_air_factor * timestep;
	// Spawn 8 whitewater particles max, depending on spawn chance
	float c = curand_uniform(&curand_states[particle_idx]);	// Random in [0.0, 1.0)
	if (c < spawn_chance) {									// If spawned at all
		unsigned int n = ceilf(c * whitewater_spawn_num);	// Map to [1, spawn_num]
		unsigned int idx_0 = new_whitewater_start_idx + atomicAdd(spawned_whitewater_counter, n);	// Start idx is buffer start + current spawn counter (pre-add)
		for(int i = 0; i < n; ++i) {
			if (idx_0 + i >= new_whitewater_max_idx) break;				// Don't spawn particles beyond max
			// Spawn whitewater trailing the fluid particle
			glm::aligned_vec3 position = particle_position - dx * (i + 1.0f);
			position = clamp(position, glm::aligned_vec3(1.0f), glm::aligned_vec3(grid_size) - 2.0f);
			whitewater_positions[idx_0 + i] = position;
			whitewater_velocities[idx_0 + i] = new_particle_velocity;
			// Make lifetime longer for clumps of whitewater, but randomize it a little (min: 1s, max: ~8s)
			whitewater_lifetimes[idx_0 + i] = n * 2.0f + c * i;
		}
	}
}


// Identical to particles G2P, except for no velocity and velocity gradient update - just uses velocity directly
__global__ void advect_whitewater(
	glm::aligned_vec3* const whitewater_positions,
	glm::aligned_vec3* const whitewater_velocities,
	GLubyte* const whitewater_types,
	float* const whitewater_lifetimes,
	const unsigned int whitewater_start_idx,
	const unsigned int whitewater_end_idx,
	const unsigned int moved_whitewater_start_idx,
	const unsigned int moved_whitewater_max_idx,
	unsigned int* const moved_whitewater_counter,
	const glm::aligned_vec3* const cells_velocities, 
	const float* const cells_masses, 
	const glm::uvec3 grid_size,
	const ParticleMaterial particles_material, 
	const float timestep,
	const glm::aligned_vec3 gravity,
	const float boundary, 
	const float boundary_elasticity)
{
	unsigned int whitewater_idx = whitewater_start_idx + threadIdx.x + blockIdx.x * blockDim.x;
	if (whitewater_idx >= whitewater_end_idx) return;
	
	glm::aligned_vec3 whitewater_position = whitewater_positions[whitewater_idx];
	glm::aligned_vec3 whitewater_velocity = whitewater_velocities[whitewater_idx];
	float whitewater_lifetime = whitewater_lifetimes[whitewater_idx];

	// Calculate weights for the neighbouring cells surrounding the whitewater's position on the grid using an interpolation function
	const glm::ivec3 cell_coords = whitewater_position;								// Truncated whitewater position is the grid coords of the enclosing Cell
	const glm::aligned_vec3 cell_dist = whitewater_position - glm::aligned_vec3(cell_coords) - 0.5f;	// whitewater distance to enclosing Cell's center (because Cell dimension is always 1)

	// Quadratic interpolation weights
	const glm::aligned_vec3 weights[3] = {
		0.5f * (0.5f - cell_dist) * (0.5f - cell_dist),		// 0.5 * (0.5 - d)^2
		0.75f - cell_dist * cell_dist,						// 0.75 - d^2
		0.5f * (0.5f + cell_dist) * (0.5f + cell_dist)		// 0.5 * (0.5 + d)^2
	};

	// 4.3: Calculate new whitewater type and (if foam) velocity
	float density = 0.0f;
	GLubyte whitewater_type = 0;
	glm::aligned_vec3 fluid_velocity (0.0f);
	unsigned int grid_size_yz = grid_size.y * grid_size.z;
	// whitewater's grid neighbourhood. 
	#pragma unroll
	for (int x = 0; x < 3; ++x) {
		#pragma unroll
		for (int y = 0; y < 3; ++y) {
			#pragma unroll
			for (int z = 0; z < 3; ++z) {
				glm::ivec3 n_cell_coords (cell_coords + glm::ivec3(x, y, z) - 1);
				glm::aligned_vec3 n_cell_dist = glm::aligned_vec3(n_cell_coords) + 0.5f - whitewater_position;	// whitewater distance to neighbouring Cell's center

				// 4.3.1: Get this cell's weighted contribution to our whitewater's new velocity
				float weight = weights[x].x * weights[y].y * weights[z].z;
				glm::aligned_vec3 weighted_velocity = weight * cells_velocities[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z];
				fluid_velocity += weighted_velocity;
				density += weight * cells_masses[n_cell_coords.x * grid_size_yz + n_cell_coords.y * grid_size.z + n_cell_coords.z];
			}
		}
	}
	// Choose whitewater type based on local fluid density
	// Spray update
	if (density < particles_material.rest_density / 4.0f) {	// Local fluid has low density => it's in the air
		whitewater_type = 1;
		whitewater_velocity += gravity * timestep;
	}
	// Bubbles update
	else if (density >= particles_material.rest_density) {	// Local fluid has density OF fluid => it's IN the fluid
		whitewater_type = 2;
		whitewater_velocity -= gravity * timestep / 4.0f;	// VERY crude approximation of buoyancy
	}
	// Foam update
	else {													// Local fluid has less density than fluid, but isn't as sparse as spray => it's on the surface
		whitewater_type = 3;
		whitewater_velocity = fluid_velocity;	// Foam is only moved by fluid
		whitewater_lifetime -= timestep;		// Only foam has lifetime advanced
		if (whitewater_lifetime <= 0.0f) return;
	}

	unsigned int moved_whitewater_idx = moved_whitewater_start_idx + atomicAdd(moved_whitewater_counter, 1);
	if (moved_whitewater_idx >= moved_whitewater_max_idx) return;
	
	// 4.4: Advect whitewater positions by their velocity (explicit integration)
	whitewater_position += whitewater_velocity * timestep;
	// Clamp whitewater to simulation domain [1, gridSize - 2], or a bit less if bubbles
	if (whitewater_type == 2) whitewater_position = glm::clamp(whitewater_position, glm::aligned_vec3(1.3f), glm::aligned_vec3(grid_size) - 2.3f);
	else whitewater_position = glm::clamp(whitewater_position, glm::aligned_vec3(1.0f), glm::aligned_vec3(grid_size) - 2.0f);
	
	// Additional predictive boundary conditions to soften velocities near edges.
	// Taken from nialltl's implementation, but added timestep scaling and boundary elasticity.
	// Might look unnatural but should improve stability.
	if (boundary > 0.0f && boundary_elasticity > 0.0f) {
		const glm::aligned_vec3 predicted_position = whitewater_position + whitewater_velocity * timestep;
		const glm::aligned_vec3 upper_boundary = glm::aligned_vec3(grid_size) - 1.0f - boundary;

		if (predicted_position.x < boundary) whitewater_velocity.x += (boundary - predicted_position.x) * boundary_elasticity;
		if (predicted_position.y < boundary) whitewater_velocity.y += (boundary - predicted_position.y) * boundary_elasticity;
		if (predicted_position.z < boundary) whitewater_velocity.z += (boundary - predicted_position.z) * boundary_elasticity;
		if (predicted_position.x > upper_boundary.x) whitewater_velocity.x += (upper_boundary.x - predicted_position.x) * boundary_elasticity;
		if (predicted_position.y > upper_boundary.y) whitewater_velocity.y += (upper_boundary.y - predicted_position.y) * boundary_elasticity;
		if (predicted_position.z > upper_boundary.z) whitewater_velocity.z += (upper_boundary.z - predicted_position.z) * boundary_elasticity;
	}
	// Write updated values to memory
	whitewater_positions[moved_whitewater_idx] = whitewater_position;
	whitewater_velocities[moved_whitewater_idx] = whitewater_velocity;
	whitewater_types[moved_whitewater_idx] = whitewater_type;
	whitewater_lifetimes[moved_whitewater_idx] = whitewater_lifetime;

	// Check for NaN values (aka if whitewater simulation broke)
	#ifdef ENABLE_ASSERTS
		assert(whitewater_position.x == whitewater_position.x);
		assert(whitewater_position.y == whitewater_position.y);
		assert(whitewater_position.z == whitewater_position.z);
	#endif
}
