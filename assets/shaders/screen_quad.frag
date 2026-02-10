#version 460 core

uniform sampler2D u_texture;

in vec2 v_UV;

out vec4 frag_color;

void main()
{
	vec4 tex_color = texture(u_texture, v_UV);
	if (tex_color.a <= 0.0) discard;
	frag_color = tex_color;
}