#version 460 core

// DIFFERENT structure than Vertex
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_velocity;
layout (location = 2) in float a_mass;
layout (location = 3) in float a_volume;
layout (location = 4) in vec3 a_color;
// TODO: Add more Particle parameters to visualize?

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;

out vec3 v_velocity;
out float v_mass;
out float v_volume;
out vec3 v_color;

void main()
{
	v_velocity = a_velocity;
	v_mass = a_mass;
	v_volume = a_volume;
	v_color = a_color;
	//gl_PointSize = 3.0;	// Clamped depending on hardware?
	gl_PointSize = 0.1 ;	// Clamped depending on hardware?
	gl_Position = u_projection_mat * u_view_mat * u_model_mat * vec4(a_position, 1.0);
}