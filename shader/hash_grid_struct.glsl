const uint HASH_GRID_LOCKED = 1u << 0;
const uint HASH_GRID_FOOD_ACTIVE = 1u << 1;
const uint HASH_GRID_DIST_TO_NEST_ACTIVE = 1u << 2;
const uint HASH_GRID_DIST_TO_FOOD_ACTIVE = 1u << 3;

struct HashGridCell
{
	uint active_flags;
	ivec2 index;
	uint food_amount;
	float distance_to_nest;
	float distance_to_nest_age;
	float distance_to_food;
	float distance_to_food_age;
};
