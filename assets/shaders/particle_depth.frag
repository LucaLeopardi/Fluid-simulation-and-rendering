#version 460 core

// Conservative depth hint: we edit gl_FragDepth, so depth optimizations are disabled.
// But by hinting that it will only be less or equal, some optimization can still be performed.
layout (depth_less) out float gl_FragDepth;

out vec4 linear_depth;

uniform mat4 u_projection_mat;
uniform float u_particle_radius;

in vec2 v_UV;
in vec3 v_view_space_center;

void main()
{	
	// View-space normal.xy derived from UV
	vec2 normal_XY = v_UV * 2.0 - 1.0;
	float r2 = dot(normal_XY, normal_XY);
	// Discard fragment outside of sphere
	if (r2 > 1.0) discard;

	float normal_Z = sqrt(1.0 - r2);
	vec3 normal = vec3(normal_XY, normal_Z);
	vec4 view_space_position = vec4(v_view_space_center + normal * u_particle_radius, 1.0);
	linear_depth = vec4(-view_space_position.z, 0.0, 0.0, 1.0);

	// NDC depth for fragment sorting
	vec4 clip_space_position = u_projection_mat * view_space_position;
	gl_FragDepth =  (clip_space_position.z / clip_space_position.w) * 0.5 + 0.5;	// NDC is [-1, 1]
}