#version 460 core

uniform int u_particle_color;

in vec3 v_velocity;

out vec4 frag_color;

void main()
{
	switch(u_particle_color) 
	{
		case 0 :	// Velocity
			float speed = length(v_velocity) / 15.0;
			frag_color = vec4(speed, speed, 1.0 + speed, 1.0);
			break;
		case 1 :	// Velocity gradient
			frag_color = vec4(abs(v_velocity.x) / 4.0, abs(v_velocity.y) / 4.0, abs(v_velocity.z) / 4.0, 1.0);
			break;
	}
}