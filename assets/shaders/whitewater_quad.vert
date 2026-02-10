#version 460 core

layout (location = 0) in vec2 a_vertex_offset;	// Instanced quads vertices
layout (location = 1) in vec3 a_position;
layout (location = 2) in uint a_type;			// 1 = Spray, 2 = Bubble, 3 = Foam
layout (location = 3) in float a_lifetime;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform float u_particle_radius;

out vec2 v_UV;
flat out uint v_type;
out float v_lifetime;
out vec3 v_view_space_center;

void main()
{
	v_UV = a_vertex_offset + 0.5;
	v_type = a_type;
	v_lifetime = a_lifetime;
	v_view_space_center = (u_view_mat * u_model_mat * vec4(a_position, 1.0)).xyz;
	gl_Position = u_projection_mat * vec4(v_view_space_center + vec3(a_vertex_offset, 0.0) * u_particle_radius, 1.0);
}