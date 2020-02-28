#include "server_manage.h"
#include <condition_variable>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../lib/TCPCLSWIN.h"
#include "class_Master.h"
#include "class_Slave.h"
#include "Server.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

int master_alive = 0;
std::mutex master_mut;
std::condition_variable master_cond;
//
// Master端监听线程
//
void master_waiter()
{
	TCPListener listener(master_port, master_backlog);
	while (true)
	{
		std::unique_lock<std::mutex> mwfl(master_mut);
		master_cond.wait(mwfl, [] {return master_alive < max_master_number; });
		mwfl.unlock();	// 收到消息后立即释放管理锁
		for (int i = 0; i < max_master_number; ++i)
		{
			MasterInfo& mi = master_pool[i];
			if (mi.lock.try_lock())	// 获取修改锁
			{
				if (!mi.p)	// 如果指针为空，是空位
				{
					TCPSession new_session = listener.accept();
					mi.ip = new_session.get_ip();
					mi.port = new_session.get_port();
					mi.p = new Master(std::move(new_session));
					std::cout << "new master connect: ip=" << mi.ip << ", port=" << mi.port << std::endl;
				}
				mi.lock.unlock();
			}
		}
	}
}

int slave_alive = 0;
std::mutex slave_mut;
std::condition_variable slave_cond;
//
// Slave端监听线程
//
void slave_waiter()
{
	TCPListener listener(slave_port, slave_backlog);
	while (true)
	{
		std::unique_lock<std::mutex> swfl(slave_mut);
		slave_cond.wait(swfl, [] {return slave_alive < max_slave_number; });
		swfl.unlock();
		for (int i = 0; i < max_slave_number; ++i)
		{
			SlaveInfo& si = slave_pool[i];
			if (si.lock.try_lock())
			{
				if (!si.p)
				{
					TCPSession new_session = listener.accept();
					si.ip = new_session.get_ip();
					si.port = new_session.get_port();
					si.p = new Slave(std::move(new_session));
					std::cout << "new slave connect: ip=" << si.ip << ", port=" << si.port << std::endl;
				}
				si.lock.unlock();
			}
		}
	}
}

//
// 服务器资源管理线程，每隔5分钟检查Master和Slave连接的有效性
//
void resource_manager()
{
	while (true)
	{
		// master_pool管理
		master_mut.lock();
		master_alive = 0;
		for (int i = 0; i < max_master_number; ++i)
		{
			MasterInfo& mi = master_pool[i];
			if (mi.lock.try_lock())
			{
				if (mi.p)
				{
					if (mi.p->alive())
					{
						++master_alive;
					}
					else
					{
						delete mi.p;
						mi.p = nullptr;
					}
				}
				mi.lock.unlock();
			}
		}
		master_mut.unlock();
		if (master_alive < max_master_number)
		{
			master_cond.notify_all();
		}
		// slave_pool管理
		slave_mut.lock();
		slave_alive = 0;
		for (int i = 0; i < max_slave_number; ++i)
		{
			SlaveInfo& si = slave_pool[i];
			if (si.lock.try_lock())
			{
				if (si.p)
				{
					if (si.p->try_own())
					{
						if (si.p->alive())
						{
							++slave_alive;
							si.p->free();
						}
						else
						{
							si.p->free();
							delete si.p;
							si.p = nullptr;
						}
					}
					else
					{
						++slave_alive;
					}
				}
				si.lock.unlock();
			}
		}
		slave_mut.unlock();
		if (slave_alive < max_slave_number)
		{
			slave_cond.notify_all();
		}
		// 检查完毕，休眠5分钟
		std::cout << "server status: " << master_alive << " master alive, " << slave_alive << " slave alive" << std::endl;
		for (int i = 0; i < 5; ++i)
		{
			Sleep(60000);
		}
	}
}
