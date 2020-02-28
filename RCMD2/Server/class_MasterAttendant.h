#pragma once
#include <thread>
#include <eztcp/eztcp.h>
#include "class_MasterPool.h"
#include "class_SlavePool.h"

class MasterAttendant
{
public:
	struct HistoryRecord
	{
		int _order_code;
		std::string _arg;
	};

public:
	MasterAttendant(
		MasterPool* master_pool_p,
		int index,
		SlavePool* slave_pool_p
	);
	~MasterAttendant();

public:
	void send_stop_signal() { _stop_signal = true; }
	bool is_ended() { return _ended_flag; }

private:
	using MLPtr = MasterPool::LockerPtr;
	using SLPtr = SlavePool::LockerPtr;

private:
	bool _stop_signal;
	bool _ended_flag;
	MasterPool* _master_pool_p;
	SlavePool* _slave_pool_p;
	std::thread _t;

private:
	void main(int master_pool_index);
	SLPtr seize_control_proc(eztcp::BlockPack& arg_pack, eztcp::Session& ms);
	void get_salve_list_proc(eztcp::Session& ms);

};
