#include "cmd_functions.h"
#include <string>
#include <iomanip>
#include <memory>
#include <eztcp/eztcp.h>
#include "rcmd_orders.h"
#include "error_classes.h"
#include "Master.h"

using namespace eztcp;
using namespace eztcp::err;
using namespace stlcli;
using namespace stlcli::err;

Session* g_to_server_p = nullptr;
BlockPack no_args;

// =============================================================================

std::ostream& operator<<(std::ostream& out, RecvPack& recv_pack)
{
	BlockPack output_pack = recv_pack;
	out << output_pack.get_buf() << std::endl;
	return out;
}

inline void send_order_and_confirm(int order_code, eztcp::MemPack& arg_pack, eztcp::Session& s)
{
	send_order(s, order_code, arg_pack);
	if (int n = recv_feedback(s))
		throw FeedbackError(n);
}

// 检查文件路径能否被以指定方式打开
bool check_file(const char* file_path_ps, const char* mode_ps)
{
	FILE* f_p;
	if (!(f_p = fopen(file_path_ps, mode_ps)))
		return false;
	fclose(f_p);
	return true;
}

void error_proc(FileIOError& e, Console& con)
{
	con.lock();
	con << ANSI_TRED << ANSI_LIGHT << "FileIO Error: " << e.what() << ANSI_DEFAULT << std::endl;
	con.unlock();
}

void error_proc(SocketError& e, Console& con)
{
	con.lock();
	con << ANSI_TRED << ANSI_LIGHT << "Socket Error: " << e.what() << "\n";
	delete g_to_server_p;
	g_to_server_p = nullptr;
	con << ANSI_TBLUE << ANSI_LIGHT << "disconnected to server\n";
	con << ANSI_DEFAULT;
	con << std::flush;
	con.unlock();
}

void error_proc(FeedbackError& e, Console& con)
{
	con.lock();
	con << ANSI_TRED << ANSI_LIGHT << "Result Error: " << e.what() << "\n";
	con << ANSI_TBLUE << ANSI_LIGHT;
	switch (e.what())
	{
	case RCMD_ORDER_ERROR:
		con << "Server: unknown order code.\n";
		break;
	case RCMD_ARGS_ERROR:
		con << "Server: invalid arguments.\n";
		break;
	case RCMD_SLAVE_LOST_ERROR:
		con << "Server: lost the slave.\n";
		break;
	case RCMD_SLAVE_BUSY_ERROR:
		con << "Server: the slave is busy.\n";
		break;
	case RCMD_NO_SLAVE_ERROR:
		con << "Server: no target selected.\n";
		break;
	default:
		con << "fatal error occured, disconnecting...\n";
		delete g_to_server_p;
		g_to_server_p = nullptr;
		con << "disconnected to server\n";
		break;
	}
	con << ANSI_DEFAULT;
	con << std::flush;
	con.unlock();
}

void error_proc(UnconnectedError& e, Console& con)
{
	con.lock();
	con << ANSI_TRED << ANSI_LIGHT << "Unconnected Error: not connect to server" << ANSI_DEFAULT << std::endl;
	con.unlock();
}

void error_proc(ArgumentError& e, Console& con)
{
	con.lock();
	con << ANSI_TRED << ANSI_LIGHT << "Argument Error: " << e.what() << ANSI_DEFAULT << std::endl;
	con.unlock();
}

void success(Console& con)
{
	con.lock();
	con << ANSI_TGREEN << ANSI_LIGHT << "success" << ANSI_DEFAULT << std::endl;
	con.unlock();
}

inline void success() { return success(g_console); }

void show_message(const std::string& msg, Console& con)
{
	con.lock();
	con << ANSI_TBLUE << ANSI_LIGHT << msg << ANSI_DEFAULT << std::endl;
	con.unlock();
}

inline void show_message(const std::string& msg) { return show_message(msg, g_console); }

// =============================================================================

DEFINE_PROC_CMD(recv_file)
{
	try
	{
		if (!g_to_server_p) throw UnconnectedError();
		std::string src_path, dsc_path;
		argi >> src_path;
		argi >> dsc_path;
		if (!check_file(dsc_path.c_str(), "wb"))
			throw FileIOError(errno);
		//
		// =============== Args Prepared ===============
		//
		MemPack arg_pack(src_path.c_str(), src_path.size() + 1);
		send_order_and_confirm(RCMD_RECV_FILE, arg_pack, *g_to_server_p);	// 1. 确认指令
		show_message("transmitting...");
		FilePack recv_file_pack(g_to_server_p->recv(), dsc_path.c_str());	// 2. 接受文件包
		//
		// ================== Succeed ==================
		//
		success();
	}
	catch (FileIOError& e)
	{
		error_proc(e, con);
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	catch (ArgumentError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(send_file)
{
	try
	{
		if (!g_to_server_p) throw UnconnectedError();
		std::string src_path, dsc_path;
		argi >> src_path;
		argi >> dsc_path;
		FilePack file_pack(src_path.c_str());
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(dsc_path.c_str(), dsc_path.size() + 1);
		send_order_and_confirm(RCMD_SEND_FILE, args_pack, *g_to_server_p);	// 1.确认指令
		show_message("transmitting...");
		g_to_server_p->send(file_pack);										// 2.发送文件包
		//
		// ================== Succeed ==================
		//
		success();
	}
	catch (FileIOError& e)
	{
		error_proc(e, con);
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	catch (ArgumentError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(execute_cmd)
{
	try
	{
		if (!g_to_server_p) throw UnconnectedError();
		std::string cmd_line;
		std::getline(argi, cmd_line);
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(cmd_line.c_str(), cmd_line.size() + 1);
		send_order_and_confirm(RCMD_EXECUTE_CMD, args_pack, *g_to_server_p);	// 1. 确认指令
		RecvPack& output_pack = g_to_server_p->recv();							// 2. 接收返回数据包
		//
		// ================== Succeed ==================
		//
		con.lock();
		con << ANSI_TBLUE << ANSI_LIGHT << "RE:\n" << ANSI_DEFAULT;
		con << output_pack;
		con << ANSI_TBLUE << ANSI_LIGHT << "[END]\n";
		con << ANSI_TGREEN << ANSI_LIGHT << "success" << ANSI_DEFAULT << std::endl;
		con.unlock();
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	catch (ArgumentError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(seize_control)
{
	try
	{
		if (!g_to_server_p) throw UnconnectedError();
		int index = -1;
		argi >> index;
		if (index == -1) throw ArgumentError("invalid index");
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(&index, sizeof(index));
		send_order_and_confirm(RCMD_SEIZE_CONTROL, args_pack, *g_to_server_p);	// 1.指令确认
		//
		// ================== Succeed ==================
		//
		success();
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	catch (ArgumentError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(list_slaves)
{
	try
	{
		if (!g_to_server_p) throw UnconnectedError();
		//
		// =============== Args Prepared ===============
		//
		send_order_and_confirm(RCMD_GET_SLAVE_LIST, no_args, *g_to_server_p);	// 1. 确认指令
		BlockPack slave_list;
		*g_to_server_p >> slave_list;											// 2. 接收slave_list;
		//
		// ================== Succeed ==================
		//
		con.lock();
		con << ANSI_TBLUE << ANSI_LIGHT << "RE:\n" << ANSI_DEFAULT;
		con << slave_list.get_buf();
		con << ANSI_TBLUE << ANSI_LIGHT << "[END]\n";
		con << ANSI_TGREEN << ANSI_LIGHT << "success" << ANSI_DEFAULT << std::endl;
		con.unlock();
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(connect_server)
{
	try
	{
		if (g_to_server_p)
		{
			delete g_to_server_p;
			show_message("disconnected from present server");
		}
		std::string ip = "";
		argi >> ip;
		unsigned short port = 0xffff;
		argi >> port;
		if (ip == "") throw ArgumentError("no ip given");
		if (port == 0xffff) throw ArgumentError("no port given");
		//
		// =============== Args Prepared ===============
		//
		con.lock();
		con << ANSI_TBLUE << ANSI_LIGHT << "connecting... (ip: " << ip << ", port: " << port << ")" << ANSI_DEFAULT << std::endl;
		con.unlock();
		g_to_server_p = new Session(ip.c_str(), port);
		send_order_and_confirm(RCMD_PULSE, no_args, *g_to_server_p);	// 1. 确认指令
		//
		// ================== Succeed ==================
		//
		success();
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	catch (ArgumentError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}

DEFINE_PROC_CMD(show_status)
{
	try
	{
		if (!g_to_server_p)
		{
			show_message("Unconnected");
			return;
		}
		//
		// =============== Args Prepared ===============
		//
		send_order_and_confirm(RCMD_PULSE, no_args, *g_to_server_p);	// 1.指令确认
		//
		// ================== Succeed ==================
		//
		con.lock();
		con << ANSI_TGREEN << ANSI_LIGHT;
		con << "Connected\n";
		con << ANSI_TBLUE << ANSI_LIGHT;
		con << "server ip:" << g_to_server_p->get_ip() << "\n";
		con << "server port:" << g_to_server_p->get_port() << "\n";
		con << ANSI_DEFAULT;
		con.unlock();
	}
	catch (SocketError& e)
	{
		error_proc(e, con);
	}
	catch (FeedbackError& e)
	{
		error_proc(e, con);
	}
	con << std::flush;
}
