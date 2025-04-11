bool is_valid_ant(uint index)
{
	return index < ANT_COUNT;
}

void clear_ant(uint index)
{
	ants[index].pos = vec2(0.0, 0.0);
	ants[index].dir = vec2(0.0, 0.0);
}
