#include "class_MasterAttendant.h"
#include <sstream>
#include <iomanip>
#include "rcmd_orders.h"
#include "Server.h"

// ===== loggings =====

void started_logging(const std::string& ip)
{
	g_console.lock();
	g_console << ANSI_TBLUE << ANSI_LIGHT << "[Master Attendant] started serving master: ip=" << ip << '\n';
	g_console << ANSI_DEFAULT << std::flush;
	g_console.unlock();
}

void error_logging(eztcp::err::SocketError& e)
{
	g_console << ANSI_TRED << ANSI_LIGHT << "[Master Attendant] SocketError: " << e.what() << '\n';
	g_console << ANSI_DEFAULT << std::flush;
	g_console.unlock();
}

void error_logging(OrderError& e)
{
	g_console.lock();
	g_console << ANSI_TRED << ANSI_LIGHT << "[Master Attendant] OrderError: " << e.what() << '\n';
	g_console << ANSI_DEFAULT << std::flush;
	g_console.unlock();
}

void master_lost_logging(int index)
{
	g_status_board.lock();
	auto& me = g_status_board._master_info[index];
	if (me._slave_index != -1)
		g_status_board._slave_info[me._slave_index]._master_index = -1;
	me = MasterInfo();
	g_status_board.unlock();
}

void change_slave_logging(int master_index, int new_index)
{
	g_status_board.lock();
	auto& me = g_status_board._master_info[master_index];
	if (me._slave_index != -1)
		g_status_board._slave_info[me._slave_index]._master_index = -1;
	g_status_board._slave_info[new_index]._master_index = master_index;
	me._slave_index = new_index;
	g_status_board.unlock();
}

// ===== cmdprocs =====

void execute_cmd_proc(eztcp::BlockPack& arg_pack, Master* m_p, Slave* s_p)
{
	eztcp::Session& ms = m_p->get_session();
	// 检查是否选中Slave
	if (!s_p)
	{
		send_feedback(ms, RCMD_NO_SLAVE_ERROR);
		return;
	}
	// 向Slave转发指令
	eztcp::Session& ss = s_p->get_session();
	send_order(ss, RCMD_EXECUTE_CMD, arg_pack);
	// 转发Slave反馈
	int n = recv_feedback(ss);
	send_feedback(ms, n);
	// 如果Slave异常则返回
	if (n)
		return;
	// 否则转发后续内容
	ms << ss.recv();
}

void send_file_proc(eztcp::BlockPack& arg_pack, Master* m_p, Slave* s_p)
{
	eztcp::Session& ms = m_p->get_session();
	// 检查是否选中Slave
	if (!s_p)
	{
		send_feedback(ms, RCMD_NO_SLAVE_ERROR);
		return;
	}
	// 向Slave转发指令
	eztcp::Session& ss = s_p->get_session();
	send_order(ss, RCMD_SEND_FILE, arg_pack);
	// 转发Slave反馈
	int n = recv_feedback(ss);
	send_feedback(ms, n);
	// 如果Slave异常则返回
	if (n)
		return;
	// 否则转发后续内容
	ss << ms.recv();
}

void recv_file_proc(eztcp::BlockPack& arg_pack, Master* m_p, Slave* s_p)
{
	eztcp::Session& ms = m_p->get_session();
	// 检查是否选中Slave
	if (!s_p)
	{
		send_feedback(ms, RCMD_NO_SLAVE_ERROR);
		return;
	}
	// 向Slave转发指令
	eztcp::Session& ss = s_p->get_session();
	send_order(ss, RCMD_EXECUTE_CMD, arg_pack);
	// 转发Slave反馈
	int n = recv_feedback(ss);
	send_feedback(ms, n);
	// 如果Slave异常则返回
	if (n)
		return;
	// 否则转发后续内容
	ms << ss.recv();
}

//==============================================================================

MasterAttendant::MasterAttendant(MasterPool* master_pool_p, int index, SlavePool* slave_pool_p) :
	_stop_signal(false),
	_ended_flag(false),
	_master_pool_p(master_pool_p),
	_slave_pool_p(slave_pool_p),
	_t(&MasterAttendant::main, this, index)
{}

MasterAttendant::~MasterAttendant()
{
	_stop_signal = true;
	_t.join();
}

void MasterAttendant::main(int master_pool_index)
{
	MLPtr ml_ptr = _master_pool_p->rent(master_pool_index);
	SLPtr sl_ptr;
	started_logging(ml_ptr->item_p()->get_ip());
	try
	{
		Master* m_p = ml_ptr->item_p();
		Slave* s_p = nullptr;
		while (!_stop_signal)
		{
			eztcp::BlockPack arg_pack;
			switch (int n = recv_order(m_p->get_session(), arg_pack))
			{
			case RCMD_PULSE:
				send_feedback(m_p->get_session(), RCMD_SUCCESS);
				break;
			case RCMD_SEIZE_CONTROL:
				sl_ptr = seize_control_proc(arg_pack, m_p->get_session());
				if (sl_ptr)
					s_p = sl_ptr->item_p();
				else
					s_p = nullptr;
				change_slave_logging(ml_ptr.index(), sl_ptr.index());
				break;
			case RCMD_GET_SLAVE_LIST:
				get_salve_list_proc(m_p->get_session());
				break;
			case RCMD_EXECUTE_CMD:
				execute_cmd_proc(arg_pack, m_p, s_p);
				break;
			case RCMD_SEND_FILE:
				send_file_proc(arg_pack, m_p, s_p);
			case RCMD_RECV_FILE:
				recv_file_proc(arg_pack, m_p, s_p);
				break;
			default:
				throw OrderError(n);
			}
		}
		ml_ptr->empty_out();
	}
	catch (eztcp::err::SocketError& e)
	{
		if (ml_ptr)
			ml_ptr->empty_out();
		if (sl_ptr)
			sl_ptr->empty_out();
		error_logging(e);
	}
	catch (OrderError& e)
	{
		if (ml_ptr)
			ml_ptr->empty_out();
		error_logging(e);
	}
	master_lost_logging(ml_ptr.index());
	_ended_flag = true;
}

// HACK: 重构代码
bool test_slave_alive(Slave& slave)
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

void reset_slave_status(int index)
{
	g_status_board.lock();
	g_status_board._slave_info[index] = SlaveInfo();
	g_status_board.unlock();
}

MasterAttendant::SLPtr MasterAttendant::seize_control_proc(eztcp::BlockPack& arg_pack, eztcp::Session& ms)
{
	int index;
	arg_pack.extract(&index, sizeof(index));
	auto sl_ptr = _slave_pool_p->rent(index, false);
	if (sl_ptr)
	{
		if (sl_ptr->item_p())
		{
			if (test_slave_alive(*sl_ptr->item_p()))
			{
				send_feedback(ms, RCMD_SUCCESS);
				return std::move(sl_ptr);
			}
			else
			{
				send_feedback(ms, RCMD_SLAVE_LOST_ERROR);
				reset_slave_status(sl_ptr.index());
			}
		}
		else
		{
			send_feedback(ms, RCMD_SLAVE_LOST_ERROR);
		}
	}
	else
	{
		send_feedback(ms, RCMD_SLAVE_BUSY_ERROR);
	}
	return SLPtr();
}

void MasterAttendant::get_salve_list_proc(eztcp::Session& ms)
{
	send_feedback(ms, RCMD_SUCCESS);
	std::ostringstream sout;
	g_status_board.lock();
	sout << "Index\tIP               Master\n\n";
	for (int i = 0; i < _slave_pool_p->capacity(); ++i)
	{
		auto& si = g_status_board._slave_info[i];
		sout << std::right << std::setw(5) << i << '\t'
			<< std::left << std::setw(17) << si._ip
			<< g_status_board._master_info[si._master_index]._ip << '\n';
	}
	g_status_board.unlock();
	std::string s = sout.str();
	eztcp::MemPack s_pack(s.c_str(), s.size() + 1);
	ms << s_pack;
}
