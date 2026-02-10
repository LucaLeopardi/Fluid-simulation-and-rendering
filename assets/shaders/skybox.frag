#version 460 core

uniform samplerCube u_cubemap;

in vec3 v_tex_coords;

out vec4 frag_color;

void main()
{
    frag_color = texture(u_cubemap, v_tex_coords);
}