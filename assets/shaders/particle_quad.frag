#version 460 core

// Conservative depth hint: we edit gl_FragDepth, so depth optimizations are disabled.
// But by hinting that it will only be less or equal, some optimization can still be performed.
layout (depth_less) out float gl_FragDepth;

out vec4 frag_color;

uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform int u_particle_color;
uniform float u_particle_radius;
uniform vec3 u_light_direction;

in vec2 v_UV;
in vec3 v_velocity;
in vec3 v_view_space_center;

void main()
{	
	/*
	// Discard fragment outside of sphere
	if (length(v_UV) > u_particle_radius) discard;
	// Calculate sphere normal from XY coordinates
	vec3 normal = normalize(vec3(v_UV, u_particle_radius * 2.0 - sqrt(dot(v_UV, v_UV))));
	// Displace fragment by normal (for depth-sorting)
	vec4 clip_space_position = u_projection_mat * vec4(v_view_space_position.xyz + normal * u_particle_radius, 1.0);
	*/
	
	// View-space normal.xy derived from UV
	vec2 normal_XY = v_UV * 2.0 - 1.0;
	float r2 = dot(normal_XY, normal_XY);
	// Discard fragment outside of sphere
	if (r2 > 1.0) discard;
	
	float normal_Z = sqrt(1.0 - r2);
	vec3 normal = vec3(normal_XY, normal_Z);
	vec4 view_space_position = vec4(v_view_space_center + normal * u_particle_radius, 1.0);
	// NDC depth for fragment sorting
	vec4 clip_space_position = u_projection_mat * view_space_position;
	gl_FragDepth = (clip_space_position.z / clip_space_position.w) * 0.5 + 0.5;	// NDC is [-1, 1]

	switch(u_particle_color) 
	{
		case 0 :	// Velocity
			float speed = length(v_velocity) / 15.0;
			frag_color = vec4(speed, speed, 1.0 + speed, 1.0);
			break;
		case 1 :	// Velocity gradient
			frag_color = vec4(abs(v_velocity.x) / 4.0, abs(v_velocity.y) / 4.0, abs(v_velocity.z) / 4.0, 1.0);
			break;
		case 2 :	// Diffuse
			vec3 view_space_light_direction = normalize((u_view_mat * vec4(u_light_direction, 0.0)).xyz);
			frag_color = vec4(vec3(0.1, 0.1, 0.8) * (max(dot(view_space_light_direction, normal), 0.0) + 0.4), 1.0);
			break;
	}
}