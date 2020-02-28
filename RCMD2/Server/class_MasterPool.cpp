#include "class_MasterPool.h"
#include "Server.h"

bool test_alive(int master_index)
{
	return g_attendant_pa[master_index]->is_ended();
}

//==============================================================================

MasterPool::MasterPool(int capacity) :
	Pool<Master>(capacity)
{}

void MasterPool::refresh_and_clean()
{
	for (int i = 0; i < capacity(); ++i)
	{
		if (auto locker_ptr = rent(i, false))
		{
			if (!test_alive(i))
				locker_ptr->empty_out();
		}
	}
}
