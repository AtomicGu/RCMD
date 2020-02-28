#include "Slave.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include "../lib/TCPCLSWIN.h"
#include "../lib/RCMDPacks.h"
#include "../lib/RCMD_MACRO.h"
#include "order_proc.h"
#include "error_classes.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

TCPCLS::TCPSession* g_to_server_p = nullptr;
std::ofstream sout("./log");

void init()
{
#ifndef HARMONY
	std::clog.rdbuf(sout.rdbuf());
#endif
}

void end()
{
	sout.close();
}

void show_start_info()
{
	std::clog << "server info: ip=" << g_server_ip_ps << ", port=" << g_server_port << std::endl;
}

bool try_connect_server()
{
	try
	{
		g_to_server_p = new TCPSession(g_server_ip_ps, g_server_port);
		return true;
	}
	catch (WSAError&)
	{
		delete g_to_server_p;
		g_to_server_p = nullptr;
		return false;
	}
}

void wait_and_proc_order()
{
	while (true)
	{
		OrderPack order_pack = g_to_server_p->recv();
		BlockPack args_pack = g_to_server_p->recv();
		switch (order_pack._code)
		{
		case RCMD_PULSE:
			pulse_proc(args_pack);
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
			send_responce(RCMD_ORDER_ERROR);
			break;
		}
	}
}

int main()
{
	init();
	show_start_info();
	while (true)
	{
		try
		{
			std::clog << "try connecting to server" << std::endl;
			if (try_connect_server())
			{
				std::clog << "connected to server" << std::endl;
				wait_and_proc_order();
			}
			else
			{
				std::clog << "fail to connect to server" << std::endl;
				std::clog << "sleep for 5 minutes" << std::endl;
				for (int i = 0; i < 5; ++i)
				{
					Sleep(60000);
				}
			}
		}
		catch (WSAError& e)
		{
			error_proc(e);
		}
	}
	end();
}
