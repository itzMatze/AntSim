const uint HASH_GRID_LOCKED = 1u << 0;
const uint HASH_GRID_ACTIVE = 1u << 1;
const uint HASH_GRID_NEST_PHEROMONE = 1u << 2;
const uint HASH_GRID_FOOD_PHEROMONE = 1u << 3;

struct HashGridCell
{
	ivec2 index;
	int food_amount;
	float pheromone_lifetime;
	float pheromone_distance;
	uint state_bits;
};

// in meters
const float HASH_GRID_CELL_LENGTH = 0.04;

// in second
const float PHEROMONE_LIFETIME = 120.0;
