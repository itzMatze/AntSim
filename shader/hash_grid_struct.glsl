const uint HASH_GRID_LOCKED = 1u << 0;
const uint HASH_GRID_ACTIVE = 1u << 1;

struct HashGridCell
{
	ivec2 index;
	uint active_flags;
	uint food_amount;
	float distance_to_nest;
	float distance_to_nest_lifetime;
	float distance_to_food;
	float distance_to_food_lifetime;
};

// decimeter resolution
const float HASH_GRID_CELL_LENGTH = 0.1;
