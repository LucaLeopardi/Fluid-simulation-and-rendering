#pragma once

#include <glm/glm.hpp>
#include <MPM/ParticleMaterial.hpp>

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	float mass;	// Constant
	float volume;
	ParticleMaterial material;
	glm::mat3 velocity_gradient;	// Affine velocity gradient

	Particle(
		const glm::vec3& p,
		const glm::vec3& v0,
		const float m,
		ParticleMaterial mat)
		:	
		position(p),
		velocity(v0),
		mass(m),
		volume(mass / mat.rest_density),
		material(mat),
		velocity_gradient(0.0f)
	{ }
};