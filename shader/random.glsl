uint rng_state;

uint PCGHashState()
{
	rng_state = rng_state * 747796405u + 2891336453u;
	uint state = rng_state;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uint PCGHash(uint seed)
{
	uint state = seed * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float random_float_state()
{
	return (float(PCGHashState()) / float(0xFFFFFFFFU));
}

float random_float(uint seed)
{
	return (float(PCGHash(seed)) / float(0xFFFFFFFFU));
}

float random_float_state(float min, float max)
{
	return (random_float_state() * (max - min) + min);
}

float random_float(uint seed, float min, float max)
{
	return (random_float(seed) * (max - min) + min);
}

int random_int_state(int min, int max)
{
	return int(random_float_state(min, max));
}

int random_int(uint seed, int min, int max)
{
	return int(random_float(seed, min, max));
}
