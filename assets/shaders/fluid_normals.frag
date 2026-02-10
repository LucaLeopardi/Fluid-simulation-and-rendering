#version 460 core

out vec4 normal;

uniform sampler2D u_depth_map;
uniform vec2 u_texel_size;
uniform mat4 u_projection_mat;

in vec2 v_UV;

// Utility function to find fragment view space position from its view-space UVs (from Sebastian Lague)
vec3 view_space_pos_from_UV(vec2 uv, float depth)
{
	return vec3(
		(uv.x * 2.0 - 1.0) * depth / u_projection_mat[0][0],
		(uv.y * 2.0 - 1.0) * depth / u_projection_mat[1][1],
		-depth
	);
}

void main()
{	
	float depth = texture(u_depth_map, v_UV).r;	// R channel is linear depth (from particle_depth.frag + blur passes)
	if (depth <= 0.0) discard;
	
	vec3 center_pos = view_space_pos_from_UV(v_UV, depth);

	// Use partial differences of position between neighboring pixels to calculate normal
	vec2 sample_uv = v_UV + vec2(u_texel_size.x, 0.0);
	vec3 sample_pos = view_space_pos_from_UV(sample_uv, textureLod(u_depth_map, sample_uv, 0.0).r);	// Explicit texture LOD to avoid implicit derivative in non-uniform control flow
	vec3 ddx = sample_pos - center_pos;

	sample_uv = v_UV + vec2(0.0, u_texel_size.y);
	sample_pos = view_space_pos_from_UV(sample_uv, textureLod(u_depth_map, sample_uv, 0.0).r);
	vec3 ddy = sample_pos - center_pos;
	
	// For edges, use difference in opposite direction
	sample_uv = v_UV - vec2(u_texel_size.x, 0.0);
	sample_pos = view_space_pos_from_UV(sample_uv, textureLod(u_depth_map, sample_uv, 0.0).r);
	vec3 ddx_opp = center_pos - sample_pos; 
	
	sample_uv = v_UV - vec2(0.0, u_texel_size.y);
	sample_pos = view_space_pos_from_UV(sample_uv, textureLod(u_depth_map, sample_uv, 0.0).r);
	vec3 ddy_opp = center_pos - sample_pos; 
	
	if (abs(ddx.z) > abs(ddx_opp.z)) ddx = ddx_opp;
	if (abs(ddy.z) > abs(ddy_opp.z)) ddy = ddy_opp;

	// Calculate normal
	vec3 normal_xyz = cross(ddx, ddy);
	normal = vec4(normalize(normal_xyz), 1.0);
}