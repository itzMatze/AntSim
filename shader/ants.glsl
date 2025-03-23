struct Ant
{
	vec2 pos;
};

void clear_ant(inout Ant ant)
{
	ant.pos = vec2(-1.5, 0.0);
}
