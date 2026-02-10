#version 460 core

uniform sampler2D u_diffuse_texture;

//in vec4 v_base_color;
in vec2 v_tex_coords;

out vec4 frag_color;

void main()
{
    frag_color = texture(u_diffuse_texture, v_tex_coords);
}