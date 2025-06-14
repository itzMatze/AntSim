struct UniformBufferData
{
	uint food_visualization_code;
	uint food_pheromone_visualization_code;
	uint nest_visualization_code;
	uint nest_pheromone_visualization_code;
	vec2 range_min;
	vec2 range_max;
	uint frame_idx;
	float frame_time;
	float total_time;
};

float get_vis_code_r(uint vis_code)
{
	return float((vis_code & 0xff000000) >> 24) / 255.0;
}

float get_vis_code_g(uint vis_code)
{
	return float((vis_code & 0x00ff0000) >> 16) / 255.0;
}

float get_vis_code_b(uint vis_code)
{
	return float((vis_code & 0x0000ff00) >> 8) / 255.0;
}

uint get_vis_code_a(uint vis_code)
{
	return (vis_code & 0x000000ff);
}
