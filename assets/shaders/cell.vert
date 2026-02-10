#version 460 core

layout (location = 0) in vec3 a_velocity;
layout (location = 1) in float a_mass;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform uvec3 u_grid_size;

out vec3 v_velocity;
out float v_mass;

void main()
{
	v_velocity = a_velocity;
	v_mass = a_mass;
	
	gl_PointSize = 3.0;	// Clamped depending on hardware?
	//gl_PointSize = a_mass + 2.0;	// Clamped depending on hardware?

	float x = floor(gl_VertexID / (u_grid_size.y * u_grid_size.z));
	float y = floor((gl_VertexID / u_grid_size.z) % u_grid_size.y);
	float z = floor(gl_VertexID % u_grid_size.z);
	gl_Position = u_projection_mat * u_view_mat * u_model_mat * vec4(x, y, z, 1.0);
}