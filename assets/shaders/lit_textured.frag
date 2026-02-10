#version 460 core

out vec4 frag_color;

uniform sampler2D u_diffuse_texture;
uniform bool u_use_texture;
uniform vec3 u_light_direction;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coords;
in vec4 v_vert_color;

#define ambient 0.5

void main()
{
	float lambert = max(dot(u_light_direction, v_normal), 0.0);
	vec4 tex_color = texture(u_diffuse_texture, v_tex_coords);
	vec3 diffuse_color = u_use_texture ? (v_vert_color * tex_color).rgb : v_vert_color.rgb;
	frag_color = vec4(diffuse_color * (lambert + ambient), 1.0);
}