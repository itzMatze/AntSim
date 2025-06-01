float get_nest_radius(int level)
{
	return pow(1.2, float(level - 1)) / 100.0;
}
