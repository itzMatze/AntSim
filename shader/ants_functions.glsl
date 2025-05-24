bool is_valid_ant(uint index)
{
	return index < ANT_COUNT;
}

#ifdef ANTS_ENABLE_WRITE
void clear_ant(uint index)
{
	ants[index].pos = vec2(0.0, 0.0);
	ants[index].dir = vec2(0.0, 0.0);
	ants[index].distance_to_poi = 0.0;
	ants[index].state_bits = 0;
}
#endif
