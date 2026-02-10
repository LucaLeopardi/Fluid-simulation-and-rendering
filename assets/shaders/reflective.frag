#version 460 core

uniform vec3 u_camera_position;
uniform samplerCube u_cubemap;

in vec3 v_position;
in vec3 v_normal;

out vec4 frag_color;

void main()
{
	vec3 direction_to_camera = normalize(v_position - u_camera_position);
	vec3 tex_coords = reflect(direction_to_camera, v_normal);
    frag_color = texture(u_cubemap, tex_coords);
}