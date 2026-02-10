#version 460 core

out vec4 frag_color;

uniform sampler2D u_depth_map;
uniform sampler2D u_thickness_map;
uniform sampler2D u_normals_map;
uniform sampler2D u_scene_texture;
uniform samplerCube u_skybox;
uniform bool u_dynamic_reflections;
uniform samplerCube u_reflections_cubemap;
uniform vec3 u_reflections_cubemap_position;
uniform float u_reflections_cubemap_size;
uniform float u_fresnel_pow;
uniform float u_refraction_strength;
uniform vec3 u_light_absorbance;
uniform mat4 u_inv_view_mat;
uniform mat4 u_projection_mat;

in vec2 v_UV;

// View-space position from view-space UVs (from Sebastian Lague)
vec3 view_space_pos_from_UV(vec2 uv, float depth)
{
	return vec3(
		(uv.x * 2.0 - 1.0) * depth / u_projection_mat[0][0],
		(uv.y * 2.0 - 1.0) * depth / u_projection_mat[1][1],
		-depth
	);
}

// Ray - Axis Aligned Bounding Box intersection (slab method from Sebastién Lagarde (no relation))
bool ray_AABB_intersection(vec3 ray_origin, vec3 ray_direction, vec3 box_center, float box_size, out vec3 out_intersection)
{
	vec3 inverse_dir = 1.0 / ray_direction;	// To handle ray_direction = 0.0, so it propagates +/-inf instead of NaN
	vec3 first_intersect = (box_center - vec3(box_size / 2.0)) * inverse_dir;
	vec3 second_intersect = (box_center + vec3(box_size / 2.0)) * inverse_dir;

	vec3 entry = min(first_intersect, second_intersect);
	vec3 exit = max(first_intersect, second_intersect);

	float dist_min = max(max(entry.x, entry.y), entry.z);
	float dist_max = min(min(exit.x, exit.y), exit.z);

	// If intervals don't overlap, there is no intersection
	if (dist_max < dist_min || dist_max < 0.0) return false;
	
	// If intersection, return hit point
	out_intersection = ray_origin + ray_direction * (dist_min >= 0.0 ? dist_min : dist_max);
	return true;
}


void main()
{
	vec4 view_space_pos = vec4(view_space_pos_from_UV(v_UV, texture(u_depth_map, v_UV).r), 1.0);
	vec3 view_direction = normalize(view_space_pos.xyz);	// Would be pos - camera_pos, but in view space camera_pos == (0, 0, 0)

	vec4 tex_normal = texture(u_normals_map, v_UV);
	if (tex_normal.a <= 0.001) discard;
	vec3 normal = normalize(tex_normal.xyz);

	// Refraction (from Nvidia's Direct3D Effects)
	float thickness = textureLod(u_thickness_map, v_UV, 0.0).r;	// Explicit texture LOD to avoid implicit derivative in non-uniform control flow
	vec3 refraction = textureLod(u_scene_texture, v_UV + normal.xy * thickness * u_refraction_strength, 0.0).rgb;
	// Light decay (Beer's law)
	float r = exp(-thickness * (u_light_absorbance.r));
	float g = exp(-thickness * (u_light_absorbance.g));
	float b = exp(-thickness * (u_light_absorbance.b));
	vec3 light_decay = vec3(r, g, b);

	// Reflection
	vec4 reflection = vec4(0.0);
	vec3 reflection_dir = reflect(view_direction, normal);
	reflection_dir = normalize((u_inv_view_mat * vec4(reflection_dir, 0.0)).xyz);	// Convert to world space
	// Dynamic reflections cubemap
	if (u_dynamic_reflections) {
		vec3 world_space_pos = (u_inv_view_mat * view_space_pos).xyz;
		vec3 cubemap_hit_pos;	// Out parameter for AABB intersection
		bool cubemap_hit = ray_AABB_intersection(world_space_pos, reflection_dir, u_reflections_cubemap_position, u_reflections_cubemap_size, cubemap_hit_pos);
		if (cubemap_hit) {
			reflection_dir = normalize(cubemap_hit_pos - u_reflections_cubemap_position);	// Parallax reflection correction
			reflection += textureLod(u_reflections_cubemap, reflection_dir, 0.0);
		} 
	}
	// Fall back to skybox reflection
	if (reflection.a < 0.001) reflection += textureLod(u_skybox, reflection_dir, 0.0);

	// Schlick approximation of Fresnel reflectivity
	// R0 = ((n1 - n2) / (n1 + n2))^2
	float fresnel_0 = 0.02;	// Pre-calculated with hardcoded IoR 1.00 for air and 1.33 for fluid
	float cos_angle = dot(-view_direction, normal);
	// R(a) = R0 + (1 - R0) * (1 - cos(a))^5
	float reflection_coeff = fresnel_0 + (1.0 - fresnel_0) * pow(1.0 - cos_angle, u_fresnel_pow);
	//reflection_coeff = clamp(reflection_coeff, 0.1, 0.9);
	float refraction_coeff = 1.0 - reflection_coeff;

	frag_color = vec4(refraction * refraction_coeff * light_decay + reflection.rgb * reflection_coeff, 1.0);
}