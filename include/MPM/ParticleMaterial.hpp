#pragma once

struct ParticleMaterial
{
	float mass;
	float rest_density;
	float dynamic_viscosity;
	float EOS_stiffness;
	float EOS_power;
	float max_negative_pressure;
	glm::vec3 color;

	ParticleMaterial(
		const float mass = 125.0f, 
		const float density = 1000.0f, 
		const float viscosity = 0.1f, 
		const float stiffness = 10.f, 
		const float power = 4.0f, 
		const float max_neg_p = -0.1f, 
		const glm::vec3& rgb = glm::vec3(0.0, 0.1, 0.9))
	:
	mass(mass),
	rest_density(density),
	dynamic_viscosity(viscosity),
	EOS_stiffness(stiffness),
	EOS_power(power),
	max_negative_pressure(max_neg_p),
	color(rgb)
	{ };
};