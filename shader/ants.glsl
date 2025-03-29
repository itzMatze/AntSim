struct Ant
{
	vec2 pos;
	vec2 dir;
};

void clear_ant(inout Ant ant)
{
	ant.pos = vec2(0.0, 0.0);
	ant.dir = vec2(0.0, 0.0);
}
