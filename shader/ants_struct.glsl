const uint ANT_TARGET_PRIORITY_MASK = (1u << 3) - 1;
const uint ANT_TARGET_NO_PRIORITY = 1u;
const uint ANT_TARGET_PHEROMONE_PRIORITY = 2u;
const uint ANT_TARGET_FOOD_NEST_PRIORITY = 3u;
const uint ANT_HAS_FOOD = 1u << 3;

struct Ant
{
	vec2 pos;
	vec2 dir;
	vec2 target;
	float distance_to_poi;
	// track how many other pheromones the ant sees to increase probability of emitting one
	float pheromone_emit_scale;
	uint state_bits;
	uint pad0;
};
