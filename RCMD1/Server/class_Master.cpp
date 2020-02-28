#include <string>
#include "class_Master.h"
#include "../lib/RCMDPacks.h"
#include "../lib/RCMD_MACRO.h"
#include "error_classes.h"
#include "Server.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

int recv_order(TCPSession& s, BlockPack& args_pack) throw(WSAError)
{
	OrderPack order_pack = s.recv();
	args_pack = s.recv();
	return order_pack._code;
}

void end_check(TCPSession& s) throw(WSAError, FeedbackError)
{
	OrderPack end_pack = s.recv();
	if (end_pack._code != RCMD_CHECK_END)
	{
		throw FeedbackError(end_pack._code);
	}
	ResultPack result_pack(RCMD_CHECK_END);
	s.send(result_pack);
}

void error_proc(WSAError& e, TCPSession& p)
{
	std::cerr << p.get_ip() << " disconnected (WSAError:" << e._code << ")\n";
	p.~TCPSession();
}

void error_proc(FeedbackError& e, TCPSession& p)
{
	std::cerr << p.get_ip() << " disconnected (FeedbackError:" << e._code << ")\n";
	p.~TCPSession();
}

// =============================================================================

void slave_deleter(Slave* p)
{
	p->free();
}

Master::Master(TCPCLS::TCPSession s) :
	_s(std::move(s)),
	_my_slave(nullptr, slave_deleter),
	_thread(std::thread(&Master::thread_main, this))
{
	_thread.detach();
}

bool Master::alive()
{
	bool locked = _alive.try_lock();
	if (locked) _alive.unlock();
	return !locked;
}

void Master::thread_main()
{
	_alive.lock();
	try
	{
		while (true)
		{
			BlockPack args_pack;
			switch (recv_order(_s, args_pack))
			{
			case RCMD_PULSE:
				pulse_proc(args_pack);
				break;
			case RCMD_GET_SLAVE_LIST:
				get_slave_list_proc(args_pack);
				break;
			case RCMD_SEIZE_CONTROL:
				seize_control_proc(args_pack);
				break;
			case RCMD_EXECUTE_CMD:
				execute_cmd_proc(args_pack);
				break;
			case RCMD_SEND_FILE:
				send_file_proc(args_pack);
				break;
			case RCMD_RECV_FILE:
				recv_file_proc(args_pack);
				break;
			default:
			{
				ResultPack order_error_pack(RCMD_ORDER_ERROR);
				_s.send(order_error_pack);
				continue;
			}
			}
		}
	}
	catch (WSAError& e)	// 这里的都是致命错误，必须断开连接
	{
		error_proc(e, _s);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, _s);
	}
	_alive.unlock();
}

// =============================================================================

//
// 线程主函数进入请求处理函数时，控制主机仍在等候就绪消息
// 任何请求处理函数在初始化后没问题后，必须调用此函数回复消息
//
void send_responce(TCPSession& s, int result_code) throw(WSAError)
{
	ResultPack order_error_pack(result_code);
	s.send(order_error_pack);
}

bool transmit_begin_check(TCPSession& s, Slave* p)
{
	if (p) return true;
	send_responce(s, RCMD_NO_SLAVE_ERROR);
	return false;
}

void transmit_end_check(TCPSession& s, Slave* p)
{
	BlockPack end_pack = s.recv();
	BlockPack result_pack;
	try
	{
		p->send_pack(end_pack);
		result_pack = p->recv_pack();
	}
	catch (WSAError)
	{
		ResultPack fail_pack(RCMD_SLAVE_LOST_ERROR);
		s.send(fail_pack);
		return;
	}
	s.send(result_pack);
}

// =============================================================================

void Master::pulse_proc(BlockPack& args_pack)
{
	send_responce(_s, RCMD_SUCCESS);
}

void Master::get_slave_list_proc(BlockPack& args_pack)
{
	send_responce(_s, RCMD_SUCCESS);
	std::string str_list;
	for (int i = 0; i < max_slave_number; ++i)
	{
		SlaveInfo& si = slave_pool[i];
		char str_num[4] = { 0 };
		sprintf(str_num, "%d", i);
		str_list += str_num;
		if (si.p)
		{
			if (si.lock.try_lock())
			{
				str_list += " ip:";
				str_list += si.ip;
				str_list += " port:";
				char str_port[6] = { 0 };
				sprintf(str_port, "%d", si.port);
				si.lock.unlock();
				str_list += str_port;
				str_list += '\n';
			}
			else
			{
				str_list += " busy\n";
			}
		}
		else
		{
			str_list += " nobody\n";
		}
	}
	MemPack output_pack(str_list.c_str(), str_list.size() + 1);
	_s.send(output_pack);
	end_check(_s);
}

void Master::seize_control_proc(BlockPack& args_pack)
{
	int index;
	args_pack.extract((char*)& index, sizeof(index));
	if (index < 0 || index > max_slave_number)
	{
		send_responce(_s, RCMD_ARGS_ERROR);
		return;
	}
	SlaveInfo& ts = slave_pool[index];
	if (ts.lock.try_lock())	// 获取SlaveInfo的资源锁
	{
		if (ts.p)	// 如果SlaveInfo的指针有效
		{
			if (ts.p->try_own())	// 尝试获取Slave的资源锁
			{
				if (ts.p->alive())	// 如果Slave的连接依然有效
				{
					_my_slave = UPSlave(ts.p, slave_deleter);	// Slave的资源锁交由智能指针管理
					send_responce(_s, RCMD_SUCCESS);
				}
				else
				{
					delete ts.p;
					ts.p = nullptr;
					send_responce(_s, RCMD_SLAVE_LOST_ERROR);
				}
			}
			else
			{
				send_responce(_s, RCMD_SLAVE_BUSY_ERROR);
			}
		}
		else
		{
			send_responce(_s, RCMD_ARGS_ERROR);
		}
		ts.lock.unlock();
	}
	else
	{
		send_responce(_s, RCMD_SLAVE_BUSY_ERROR);
	}
}

void Master::execute_cmd_proc(BlockPack& args_pack)
{
	if (transmit_begin_check(_s, _my_slave.get()))	// 0.转发可行性检查
	{
		OrderPack transmit_order(RCMD_EXECUTE_CMD);
		_my_slave->send_pack(transmit_order);		// 1.转发指令包 m->s
		_my_slave->send_pack(args_pack);			// 2.转发参数包 m->s
		_s.send(_my_slave->recv_pack());			// 3.转发响应包 m<-s
		_s.send(_my_slave->recv_pack());			// 4.转发附加数据包 m<-s
		transmit_end_check(_s, _my_slave.get());	// 5.转发结束确认 m<->s
	}
}

void Master::send_file_proc(BlockPack& args_pack)
{
	if (transmit_begin_check(_s, _my_slave.get()))	// 0.转发可行性检查
	{
		OrderPack transmit_order(RCMD_SEND_FILE);
		_my_slave->send_pack(transmit_order);		// 1.转发指令包 m->s
		_my_slave->send_pack(args_pack);			// 2.转发参数包 m->s
		_s.send(_my_slave->recv_pack());			// 3.转发响应包 m<-s
		_my_slave->send_pack(_s.recv());			// 4.转发附加数据包 m->s
		transmit_end_check(_s, _my_slave.get());	// 5.转发结束确认 m<->s
	}
}

void Master::recv_file_proc(BlockPack& args_pack)
{
	if (transmit_begin_check(_s, _my_slave.get()))	// 0.转发可行性检查
	{
		OrderPack transmit_order(RCMD_RECV_FILE);
		_my_slave->send_pack(transmit_order);		// 1.转发指令包 m->s
		_my_slave->send_pack(args_pack);			// 2.转发参数包 m->s
		_s.send(_my_slave->recv_pack());			// 3.转发响应包 m<-s
		_s.send(_my_slave->recv_pack());			// 4.转发附加数据包 m<-s
		transmit_end_check(_s, _my_slave.get());	// 5.转发结束确认 m<->s
	}
}

// HACK:在Slave发送出错时仍保留Master的连接
