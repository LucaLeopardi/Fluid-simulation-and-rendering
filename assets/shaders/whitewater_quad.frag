#version 460 core

// Conservative depth hint: we edit gl_FragDepth, so depth optimizations are disabled.
// But by hinting that it will only be less or equal, some optimization can still be performed.
layout (depth_less) out float gl_FragDepth;

out vec4 frag_color;

uniform samplerCube u_ambient;
uniform mat4 u_inv_view_mat;
uniform mat4 u_projection_mat;
uniform float u_particle_radius;

in vec2 v_UV;
flat in uint v_type;	// 1 = Spray, 2 = Bubble, 3 = Foam
in float v_lifetime;
in vec3 v_view_space_center;

void main()
{
	// View-space normal.xy derived from UV
	vec2 normal_XY = v_UV * 2.0 - 1.0;
	float r2 = dot(normal_XY, normal_XY);

	// Bubbles
	if (v_type == 2 && (r2 > min(0.5, v_lifetime / 12.0) || r2 < min(0.45, v_lifetime / 12.0 - 0.05))) discard;	// Discard fragment outside of CIRCLE
	// Spray and foam 
	else if (r2 > min(1.0, v_lifetime / 6.0)) discard;															// Discard fragment outside of sphere

	float normal_Z = sqrt(1.0 - r2);
	vec3 normal = normalize(vec3(normal_XY, normal_Z));
	vec4 view_space_position = vec4(v_view_space_center + normal * u_particle_radius, 1.0);
	view_space_position.z += u_particle_radius / 10.0;	// Move A BIT towards camera so they're less obscured by fluid particles in the same position
	// NDC depth for fragment sorting
	vec4 clip_space_position = u_projection_mat * view_space_position;
	gl_FragDepth = (clip_space_position.z / clip_space_position.w) * 0.5 + 0.5;	// NDC is [-1, 1]
	
	vec3 normal_world_space = normalize(mat3(u_inv_view_mat) * normal);	// Convert to world space
	vec3 ambient = textureLod(u_ambient, normal_world_space, 0.0).rgb;

	frag_color = vec4(vec3(1.0, 1.0, 1.0) * 0.6 + ambient * 0.4, 1.0);
}