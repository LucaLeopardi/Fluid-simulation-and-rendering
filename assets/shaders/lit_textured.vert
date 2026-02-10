#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;
layout (location = 3) in vec4 a_vert_color;

uniform mat3 u_normal_mat;
uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_tex_coords;
out vec4 v_vert_color;

void main()
{
	v_position = a_position;
	v_normal = normalize(u_normal_mat * a_normal);
	v_tex_coords = a_tex_coords;
	v_vert_color = a_vert_color;
	gl_Position = u_projection_mat * u_view_mat * u_model_mat * vec4(a_position, 1.0);
}