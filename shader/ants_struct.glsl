const uint ANT_HAS_FOOD = 1u << 0;

struct Ant
{
	vec2 pos;
	vec2 dir;
	float distance_to_poi;
	uint state_bits;
};

struct Nest
{
	vec2 pos;
	float radius;
};
