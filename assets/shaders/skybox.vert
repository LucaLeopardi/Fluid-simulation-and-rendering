#version 460 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;

out vec3 v_tex_coords;

void main()
{
	v_tex_coords = a_position;
	gl_Position = (u_projection_mat * u_view_mat * vec4(a_position, 1.0)).xyww;	// Z position set to 1.0 in NDC 
}