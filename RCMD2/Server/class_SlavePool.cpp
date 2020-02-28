#include "class_SlavePool.h"
#include "rcmd_orders.h"

bool test_alive(Slave& slave)
{
	static eztcp::BlockPack no_args;	// HACK: 使用静止no_args包是否安全
	try
	{
		send_order(slave.get_session(), RCMD_PULSE, no_args);
		if (recv_feedback(slave.get_session()))
			return false;
	}
	catch (eztcp::err::SocketError& e)
	{
		return false;
	}
	return true;
}

//==============================================================================

SlavePool::SlavePool(int capacity) :
	Pool(capacity)
{}

void SlavePool::refresh_and_clean()
{
	for (int i = 0; i < capacity(); ++i)
	{
		if (auto locker_ptr = rent(i, false))
		{
			if (!test_alive(*locker_ptr->item_p()))
				locker_ptr->empty_out();
		}
	}
}
