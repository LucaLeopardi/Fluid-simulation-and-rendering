#version 460 core

in vec3 v_velocity;
in float v_mass;

out vec4 frag_color;

void main()
{
	float speed = length(v_velocity);
	frag_color = vec4(speed, speed, speed, 0.1 + v_mass / 250.0);
	//frag_color = vec4(1.0, 1.0, 0.0, 1.0);
}