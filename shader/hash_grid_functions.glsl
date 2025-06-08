bool is_valid_hash_grid(uint index)
{
	return index < HASH_GRID_CAPACITY;
}

HashGridCell get_clear_hash_grid_cell()
{
	HashGridCell cell;
	cell.index = ivec2(0, 0);
	cell.food_amount = 0;
	cell.pheromone_lifetime = 0.0;
	cell.pheromone_distance = 0.0;
	cell.state_bits = 0;
	return cell;
}

#ifdef HASH_GRID_ENABLE_WRITE
void clear_hash_grid_cell(uint index)
{
	hash_grid[index] = get_clear_hash_grid_cell();
}
#endif

// http://burtleburtle.net/bob/hash/integer.html
uint hash_jenkins32(uint a)
{
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

uint hash_index(ivec2 index)
{
	return hash_jenkins32(uint(index.x ^ 0xcf5ae914)) ^ hash_jenkins32(uint(index.y));
}

uint get_hash_grid_slot(ivec2 index)
{
	const uint hash = hash_index(index);
	return hash % HASH_GRID_CAPACITY;
}

#ifdef HASH_GRID_ENABLE_WRITE
int try_acquire_hash_grid_cell_index(ivec2 index)
{
	const int slot = int(get_hash_grid_slot(index));
	for (int bucket_offset = 0; bucket_offset < 1 && slot < HASH_GRID_CAPACITY; bucket_offset++)
	{
		const int hash_grid_index = slot + bucket_offset;
		const uint state_bits = hash_grid[hash_grid_index].state_bits;
		if ((state_bits & HASH_GRID_LOCKED) != 0u) return -1;
		if (atomicCompSwap(hash_grid[hash_grid_index].state_bits, state_bits, state_bits | HASH_GRID_LOCKED) != state_bits) return -1;
		if ((hash_grid[hash_grid_index].state_bits & HASH_GRID_ACTIVE) == 0)
		{
			hash_grid[hash_grid_index].state_bits = HASH_GRID_ACTIVE;
			hash_grid[hash_grid_index].index = index;
			return hash_grid_index;
		}
		else if (hash_grid[hash_grid_index].index == index) return hash_grid_index;
    atomicAnd(hash_grid[hash_grid_index].state_bits, ~HASH_GRID_LOCKED);
	}
	return -1;
}
#endif

int try_acquire_hash_grid_cell_index_const(ivec2 index)
{
	const int slot = int(get_hash_grid_slot(index));
	for (int bucket_offset = 0; bucket_offset < 1 && slot < HASH_GRID_CAPACITY; bucket_offset++)
	{
		const int hash_grid_index = slot + bucket_offset;
		uint state_bits = hash_grid[hash_grid_index].state_bits;
		if ((state_bits & HASH_GRID_LOCKED) != 0u || (state_bits & HASH_GRID_ACTIVE) == 0u) return -1;
		if (hash_grid[hash_grid_index].index == index) return hash_grid_index;
	}
	return -1;
}

ivec2 get_hash_grid_pos(vec2 position)
{
	return ivec2(floor(position / HASH_GRID_CELL_LENGTH));
}

vec2 get_hash_grid_cell_pos(ivec2 hash_grid_pos)
{
	return vec2(hash_grid_pos) * HASH_GRID_CELL_LENGTH + HASH_GRID_CELL_LENGTH / 2.0;
}
