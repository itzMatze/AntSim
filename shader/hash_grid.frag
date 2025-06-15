#version 460

layout(location = 0) in vec2 frag_tex;

layout(location = 0) out vec4 out_color;

#extension GL_GOOGLE_include_directive: require
#include "uniform_buffer_struct.glsl"
#include "colormaps.glsl"
#include "hash_grid_struct.glsl"
#include "ants_struct.glsl"
#include "nest_struct.glsl"
#include "nest_functions.glsl"

layout(constant_id = 0) const uint HASH_GRID_CAPACITY = 1;

layout(binding = 0) buffer HashGridBuffer {
	HashGridCell hash_grid[];
};

layout(binding = 1) buffer NestBuffer {
	Nest nest;
};

layout(binding = 99) uniform UniformBuffer {
	UniformBufferData ub;
};

#include "hash_grid_functions.glsl"

vec4 colormap(float t, uint vis_code)
{
	uint colormap_index = get_vis_code_a(vis_code);
	if (colormap_index == 1) return vec4(viridis(t), 1.0);
	else if (colormap_index == 2) return vec4(plasma(t), 1.0);
	else if (colormap_index == 3) return vec4(magma(t), 1.0);
	else if (colormap_index == 4) return vec4(inferno(t), 1.0);
	else if (colormap_index == 5)
	{
		vec3 output_color = vec3(get_vis_code_r(vis_code), get_vis_code_g(vis_code), get_vis_code_b(vis_code));
		return vec4(output_color, 1.0);
	}
}

void main()
{
	vec2 pos = frag_tex * (ub.range_max - ub.range_min) + ub.range_min;
	ivec2 hash_grid_pos = get_hash_grid_pos(pos);
	if (distance(pos, nest.pos) < get_nest_radius(nest.level) && get_vis_code_a(ub.nest_vis_code) > 0)
	{
		const uint required_food = get_nest_next_level_food_amount(nest.level) - get_nest_next_level_food_amount(nest.level - 1);
		out_color = colormap(float(nest.food_amount - get_nest_next_level_food_amount(nest.level - 1)) / float(required_food), ub.nest_vis_code);
		return;
	}
	int hash_grid_index = get_hash_grid_cell_index_const(hash_grid_pos);
	if (hash_grid_index >= 0)
	{
		HashGridCell cell = hash_grid[hash_grid_index];
		if (cell.food_amount > 0 && get_vis_code_a(ub.food_vis_code) > 0) out_color = colormap(float(cell.food_amount) / 255.0, ub.food_vis_code);
		else if ((cell.state_bits & HASH_GRID_FOOD_PHEROMONE) != 0 && get_vis_code_a(ub.food_pheromone_vis_code) > 0)
		{
			out_color = colormap((PHEROMONE_LIFETIME - cell.pheromone_lifetime) / PHEROMONE_LIFETIME, ub.food_pheromone_vis_code);
		}
		else if ((cell.state_bits & HASH_GRID_NEST_PHEROMONE) != 0 && get_vis_code_a(ub.nest_pheromone_vis_code) > 0)
		{
			out_color = colormap((PHEROMONE_LIFETIME - cell.pheromone_lifetime) / PHEROMONE_LIFETIME, ub.nest_pheromone_vis_code);
		}
		else out_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		out_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
