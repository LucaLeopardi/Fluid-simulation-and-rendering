#version 460 core

layout (location = 0) in vec2 a_vertex_offset;	// Instanced quads vertices
layout (location = 1) in vec3 a_position;
layout (location = 2) in vec3 a_velocity;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform float u_particle_radius;

out vec2 v_UV;
out vec3 v_velocity;
out vec3 v_view_space_center;

void main()
{
	/*
	v_UV = a_vertex_offset;
	v_velocity = a_velocity;
	vec3 camera_right = normalize(vec3(u_view_mat[0][0], u_view_mat[1][0], u_view_mat[2][0]));
	vec3 camera_up = normalize(vec3(u_view_mat[0][1], u_view_mat[1][1], u_view_mat[2][1]));
	vec3 offset_position = a_position + a_vertex_offset.x * camera_right + a_vertex_offset.y * camera_up;
	v_view_space_position = u_view_mat * u_model_mat * vec4(offset_position, 1.0);
	gl_Position = u_projection_mat * v_view_space_position;
	*/
	
	v_UV = a_vertex_offset + 0.5;
	v_velocity = a_velocity;
	v_view_space_center = (u_view_mat * u_model_mat * vec4(a_position, 1.0)).xyz;
	gl_Position = u_projection_mat * vec4(v_view_space_center + vec3(a_vertex_offset, 0.0) * u_particle_radius, 1.0);
}