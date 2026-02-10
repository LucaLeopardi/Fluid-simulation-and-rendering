#version 460 core

layout (location = 0) in vec3 a_position;
//layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;
//layout (location = 3) in vec4 a_base_color;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;

//out vec4 v_base_color;
out vec2 v_tex_coords;

void main()
{
	//v_base_color = a_base_color;
	v_tex_coords = a_tex_coords;
	gl_Position = u_projection_mat * u_view_mat * u_model_mat * vec4(a_position, 1.0);
}