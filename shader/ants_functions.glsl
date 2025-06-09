bool is_valid_ant(uint index)
{
	return index < ANT_COUNT;
}

#ifdef ANTS_ENABLE_WRITE
void clear_ant(uint index)
{
	ants[index].pos = vec2(0.0, 0.0);
	ants[index].dir = vec2(0.0, 0.0);
	ants[index].target = vec2(0.0, 0.0);
	ants[index].distance_to_poi = 0.0;
	ants[index].state_bits = 0;
}
#endif

void ant_set_target_priority(inout uint state_bits, uint priority)
{
	state_bits &= ~ANT_TARGET_PRIORITY_MASK;
	state_bits |= priority;
}
