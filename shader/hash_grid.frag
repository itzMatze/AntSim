#version 460

layout(location = 0) in vec2 frag_tex;

layout(location = 0) out vec4 out_color;

#extension GL_GOOGLE_include_directive: require
#include "colormaps.glsl"
#include "hash_grid_struct.glsl"

layout(constant_id = 0) const uint HASH_GRID_CAPACITY = 1;

layout(binding = 0) buffer HashGridBuffer {
	HashGridCell hash_grid[];
};

#include "hash_grid_functions.glsl"

struct PushConstants
{
	vec2 range_min;
	vec2 range_max;
};

layout(push_constant) uniform PushConstant { PushConstants pc; };

void main()
{
	ivec2 hash_grid_pos = get_hash_grid_pos(frag_tex * (pc.range_max - pc.range_min) + pc.range_min);
	int hash_grid_index = try_acquire_hash_grid_cell_index_const(hash_grid_pos);
	if (hash_grid_index >= 0)
	{
		float distance_to_nest = hash_grid[hash_grid_index].distance_to_nest;
		out_color = vec4(viridis(distance_to_nest / 10.0), 1.0);
	}
	else
	{
		out_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
