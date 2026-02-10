#version 460 core

out vec4 frag_color;

#define THRESHOLD 0.001
#define MAX_BLUR_RADIUS 10

uniform bool u_is_vertical;
uniform sampler2D u_texture;	// RGB channels are the vector to blur
uniform sampler2D u_depth_map;	// R channel is linearized depth
uniform vec2 u_texel_size;
uniform float u_blur_radius_world_size;
uniform float u_blur_weights[MAX_BLUR_RADIUS + 1];
uniform float u_blur_depth;
uniform int u_camera_width;
uniform float u_camera_hor_scale;

in vec2 v_UV;

void main()
{
	float depth = texture(u_depth_map, v_UV).r;
	if (depth < THRESHOLD) discard;

	// Convert kernel's world size to screen space size
	// Formula taken from Sebastian Lague, who took it from Freya Holmér
	float pixels_per_meter = (u_camera_width * u_camera_hor_scale) / (2 * depth);
	int radius = int(ceil(u_blur_radius_world_size * pixels_per_meter));
	// Clamp to [2, max]
	radius = min(radius, MAX_BLUR_RADIUS);
	radius = max(radius, 2);

	// 1D blur
	vec3 sum = vec3(0.0);
	float weights_sum = 0.0;

	vec4 center = textureLod(u_texture, v_UV, 0.0);
	if (center.a > THRESHOLD) {
		sum += center.rgb * u_blur_weights[0];
		weights_sum += u_blur_weights[0];
	}

	for (int i = 1; i <= radius; ++i) {
		// Sample either vertically or horizontally
		float i_f = float(i);
		vec2 uv_dxdy = u_is_vertical ? vec2(0.0, i_f * u_texel_size.y) : vec2(i_f * u_texel_size.x, 0.0);

		// Up/right sample
		vec4 sample_value = textureLod(u_texture, v_UV + uv_dxdy, 0.0);	// Explicit texture LOD to avoid implicit derivative in non-uniform control flow
		float weight = u_blur_weights[i];
		if (sample_value.a < THRESHOLD) 
			continue;
		if (u_blur_depth > 0.0) {
			float depth_diff = depth - textureLod(u_depth_map, v_UV + uv_dxdy, 0.0).r;
			float depth_weight = exp(-depth_diff * depth_diff / (2.0 * u_blur_depth * u_blur_depth));
			weight *= depth_weight;	// Depth difference weight, to maintain edge detail
		}
		sum += sample_value.rgb * weight;
		weights_sum += weight;

		// Down/left sample
		sample_value = textureLod(u_texture, v_UV - uv_dxdy, 0.0);	// Explicit texture LOD to avoid implicit derivative in non-uniform control flow
		weight = u_blur_weights[i];
		if (sample_value.a < THRESHOLD) 
			continue;
		if (u_blur_depth > 0.0) {
			float depth_diff = depth - textureLod(u_depth_map, v_UV - uv_dxdy, 0.0).r;
			float depth_weight = exp(-depth_diff * depth_diff / (2.0 * u_blur_depth * u_blur_depth));
			weight *= depth_weight;	// Depth difference weight, to maintain edge detail
		}
		sum += sample_value.rgb * weight;
		weights_sum += weight;
	}

	if (weights_sum > 0.0) frag_color = vec4(sum / weights_sum, 1.0);
	else discard;
}