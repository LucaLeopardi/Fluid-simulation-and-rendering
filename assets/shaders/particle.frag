#version 460 core

in vec3 v_velocity;
in float v_mass;
in float v_volume;
in vec3 v_color;

out vec4 frag_color;

void main()
{
	frag_color = vec4(0.0, 0.0, 1.0, 1.0);
	//frag_color = vec4(v_color, 1.0);
	//frag_color = vec4(abs(v_velocity.x), abs(v_velocity.y), abs(v_velocity.z), 1.0);
	//float speed = length(v_velocity) / 10.0;
	//frag_color = vec4(speed, speed, 1.0 + speed, 1.0);
}