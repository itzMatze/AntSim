float get_nest_radius(int level)
{
	return pow(1.2, float(level - 1)) / 100.0 + 0.04;
}

uint get_nest_next_level_food_amount(int level)
{
	return (1u << level);
}
