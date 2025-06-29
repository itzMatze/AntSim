#version 460

#extension GL_GOOGLE_include_directive: require
#include "uniform_buffer_struct.glsl"
#include "ants_struct.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 1) in flat int in_ant_id;
layout(location = 0) out vec4 out_color;

layout(binding = 0) buffer AntBuffer {
	Ant ants[];
};

layout(binding = 99) uniform UniformBuffer {
	UniformBufferData ub;
};

vec3 get_vis_code_color(uint vis_code)
{
	return vec3(get_vis_code_r(vis_code), get_vis_code_g(vis_code), get_vis_code_b(vis_code));
}

void main()
{
	Ant ant = ants[in_ant_id];
	float leading_color_weight = 0.0;
	if (in_uv.x + in_uv.y > 0.0)
	{
		leading_color_weight = smoothstep(radians(45.0), radians(-15.0), acos(dot(normalize(vec2(1.0, 1.0)), normalize(in_uv))));
	}

	if ((ant.state_bits & ANT_HAS_FOOD) == 0)
	{
		out_color = vec4(mix(get_vis_code_color(ub.ant_wo_food_vis_code), get_vis_code_color(ub.ant_wo_food_dir_vis_code), leading_color_weight), 1.0);
	}
	else
	{
		out_color = vec4(mix(get_vis_code_color(ub.ant_w_food_vis_code), get_vis_code_color(ub.ant_w_food_dir_vis_code), leading_color_weight), 1.0);
	}
}
