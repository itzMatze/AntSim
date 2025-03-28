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

float pcg_random_state()
{
	return (float(PCGHashState()) / float(0xFFFFFFFFU));
}

float pcg_random(uint seed)
{
	return (float(PCGHash(seed)) / float(0xFFFFFFFFU));
}

float pcg_random_state(float min, float max)
{
	return (pcg_random_state() * (max - min) + min);
}

float pcg_random(uint seed, float min, float max)
{
	return (pcg_random(seed) * (max - min) + min);
}
