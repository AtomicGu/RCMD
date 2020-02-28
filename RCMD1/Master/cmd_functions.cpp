#include "cmd_functions.h"
// Additional Headers
#include <string>
#include <memory>
#include "../lib/TCPCLSWIN.h"
#include "../lib/RCMD_MACRO.h"
#include "../lib/RCMDPacks.h"
#include "error_classes.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

TCPSession* to_server = nullptr;

BlockPack no_args;

// =============================================================================

std::ostream& operator<<(std::ostream& out, RecvPack& output_pack)
{
	size_t ptr_size = output_pack.size() - output_pack.position();
	std::unique_ptr<char> output_buffer(new char[ptr_size]);
	output_pack.extract(output_buffer.get(), ptr_size);
	out << output_buffer;
	return out;
}

void send_order(int order_code, TCPPackage& args_pack) throw(WSAError, ResultError)
{
	OrderPack order_pack(order_code);
	to_server->send(order_pack);
	to_server->send(args_pack);
	ResultPack result_pack = to_server->recv();
	if (result_pack._code) throw ResultError(result_pack._code);
}

void end_check() throw(WSAError, ResultError)
{
	OrderPack end_pack(RCMD_CHECK_END);
	to_server->send(end_pack);
	ResultPack result_pack = to_server->recv();
	if (result_pack._code != RCMD_CHECK_END)
	{
		throw ResultError(result_pack._code);
	}
}

void error_proc(FileError& e, Console* p_csl)
{
	p_csl->_out << "FAIL (FileError:" << e._code << ")\n";
}

void error_proc(WSAError& e, Console* p_csl)
{
	p_csl->_out << "FAIL (WSAError:" << e._code << ")\n";
	delete to_server;
	to_server = nullptr;
	p_csl->_out << "disconnected to server\n";
}

void error_proc(ArgsError& e, Console* p_csl)
{
	p_csl->_out << "FAIL (ArgsError:" << e._hint << ")\n";
}

void error_proc(ResultError& e, Console* p_csl)
{
	p_csl->_out << "FAIL (ResultError:" << e._code << ")\n";
	switch (e._code)
	{
	case RCMD_ORDER_ERROR:
		p_csl->_out << "server: unkown order code.\n";
		break;
	case RCMD_ARGS_ERROR:
		p_csl->_out << "server: arguments invalid.\n";
		break;
	case RCMD_SLAVE_LOST_ERROR:
		p_csl->_out << "server: lost the slave.\n";
		break;
	case RCMD_SLAVE_BUSY_ERROR:
		p_csl->_out << "server: the slave is busy.\n";
		break;
	case RCMD_NO_SLAVE_ERROR:
		p_csl->_out << "server: not select target.\n";
		break;
	default:
		p_csl->_out << "fatal error occurs, disconnected to server.\n";
		delete to_server;
		to_server = nullptr;
		p_csl->_out << "disconnected to server\n";
		break;
	}
}

void error_proc(UnconnectedError& e, Console* p_csl)
{
	p_csl->_out << "FAIL (UnconnectedError:not connect to server)\n";
}

// =============================================================================

void recv_file(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server) throw UnconnectedError();
		std::string src_path, dsc_path;
		args_in >> src_path;
		args_in >> dsc_path;
		auto deleter = [](FILE* pf)->void {fclose(pf); };
		std::unique_ptr<FILE, decltype(deleter)> p_f(fopen(dsc_path.c_str(), "wb"), deleter);
		if (!p_f) throw FileError(-1);
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(src_path.c_str(), src_path.size() + 1);
		send_order(RCMD_RECV_FILE, args_pack);					// 1.指令确认
		RecvPack& recv_file = to_server->recv();				// 2.接受文件包
		{
			static char buffer[8192];
			while (size_t n = recv_file.extract(buffer, 8192))
			{
				fwrite(buffer, 1, n, p_f.get());
			}
		}
		end_check();											// 3.结束确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "transmission succeed" << std::endl;
	}
	catch (FileError& e)
	{
		error_proc(e, p_csl);
		p_csl->_out << "fail to open file to write\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ArgsError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}

void send_file(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server) throw UnconnectedError();
		std::string src_path, dsc_path;
		args_in >> src_path;
		args_in >> dsc_path;
		FilePack file_pack(src_path.c_str());
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(dsc_path.c_str(), dsc_path.size() + 1);
		send_order(RCMD_SEND_FILE, args_pack);					// 1.指令确认
		p_csl->_out << "transmission started" << std::endl;
		to_server->send(file_pack);								// 2.发送文件包
		end_check();											// 3.结束确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "transmission succeed" << std::endl;
	}
	catch (FileError& e)
	{
		error_proc(e, p_csl);
		p_csl->_out << "fail to open source file\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ArgsError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}

void execute_cmd(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server) throw UnconnectedError();
		std::string cmd_line;
		std::getline(args_in, cmd_line);
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(cmd_line.c_str(), cmd_line.size() + 1);
		send_order(RCMD_EXECUTE_CMD, args_pack);	// 1.指令确认
		RecvPack& output_pack = to_server->recv();	// 2.接受结束包的附加数据包
		p_csl->_out << output_pack << "\n";
		end_check();								// 3.结束确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "SUCCEED\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}

void seize_control(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server) throw UnconnectedError();
		int index = -1;
		args_in >> index;
		if (index == -1) throw ArgsError("invalid index");
		//
		// =============== Args Prepared ===============
		//
		MemPack args_pack(&index, sizeof(index));
		send_order(RCMD_SEIZE_CONTROL, args_pack);	// 1.指令确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "SUCCEED\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ArgsError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}

void list_slaves(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server) throw UnconnectedError();
		//
		// =============== Args Prepared ===============
		//
		send_order(RCMD_GET_SLAVE_LIST, no_args);	// 1.指令确认
		RecvPack& output_pack = to_server->recv();	// 2.接收返回包
		p_csl->_out << output_pack << std::endl;
		end_check();
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "SUCCEED\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (UnconnectedError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}

void connect_server(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (to_server) delete to_server;
		std::string ip = "";
		args_in >> ip;
		unsigned short port = -1;
		args_in >> port;
		if (ip == "") throw ArgsError("no ip given");
		if (port == (unsigned short)-1) throw ArgsError("no port given");
		//
		// =============== Args Prepared ===============
		//
		p_csl->_out << "connecting (ip:" << ip << ", port:" << port << ")\n";
		to_server = new TCPSession(ip.c_str(), port);
		send_order(RCMD_PULSE, no_args);	// 1.指令确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "SUCCEED\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
		p_csl->_out << "connect failed\n";
	}
	catch (ArgsError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
		p_csl->_out << "connect failed\n";
	}
	p_csl->_out << std::flush;
}

void show_status(Console* p_csl, std::istream& args_in)
{
	try
	{
		if (!to_server)
		{
			p_csl->_out << "unconnected" << std::endl;
			return;
		}
		//
		// =============== Args Prepared ===============
		//
		send_order(RCMD_PULSE, no_args);	// 1.指令确认
		//
		// ================== Succeed ==================
		//
		p_csl->_out << "connected\n";
		p_csl->_out << "server ip:" << to_server->get_ip() << "\n";
		p_csl->_out << "server port:" << to_server->get_port() << "\n";
	}
	catch (WSAError& e)
	{
		error_proc(e, p_csl);
	}
	catch (ResultError& e)
	{
		error_proc(e, p_csl);
	}
	p_csl->_out << std::flush;
}
