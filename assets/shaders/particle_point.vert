#version 460 core

layout (location = 1) in vec3 a_position;
layout (location = 2) in vec3 a_velocity;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform float u_particle_radius;

out vec3 v_velocity;

void main()
{
	v_velocity = a_velocity;
	gl_PointSize = u_particle_radius * 20.0 ;	// Clamped depending on hardware?
	gl_Position = u_projection_mat * u_view_mat * u_model_mat * vec4(a_position, 1.0);
}