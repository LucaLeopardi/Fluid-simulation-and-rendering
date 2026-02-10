#version 460 core

//uniform sampler2D u_diffuseTexture;

in vec4 v_base_color;

out vec4 frag_color;

void main()
{
    frag_color = v_base_color;
}