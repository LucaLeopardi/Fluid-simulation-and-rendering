#version 460 core

out vec4 thickness;

uniform float u_particle_thickness;

in vec2 v_UV;
in vec3 v_velocity;				// Unused
in vec3 v_view_space_center;	// Unused

void main()
{	
	// View-space normal.xy derived from UV
	vec2 normal_XY = v_UV * 2.0 - 1.0;
	float r2 = dot(normal_XY, normal_XY);
	// Discard fragment outside of sphere
	if (r2 > 1.0) thickness = vec4(0.0);
	else thickness = vec4((2.0 - r2) * u_particle_thickness, 0.0, 0.0, 1.0);	// Additive rendering
}