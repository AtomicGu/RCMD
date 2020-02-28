#include "order_proc.h"
#include <string>
#include <memory>
#include "../lib/RCMDPacks.h"
#include "../lib/RCMD_MACRO.h"
#include "Slave.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

//
// 指令处理函数应该首先调用此函数返回指令接受情况
//
void send_responce(int result_code)
{
	ResultPack result_pack(result_code);
	g_to_server_p->send(result_pack);
}

void end_check()
{
	OrderPack end_pack = g_to_server_p->recv();
	if (end_pack._code != RCMD_CHECK_END)
	{
		ResultPack fail_pack(RCMD_ARGS_ERROR);
		g_to_server_p->send(fail_pack);
	}
	ResultPack result_pack(RCMD_CHECK_END);
	g_to_server_p->send(result_pack);
}

std::string exe_cmd(std::string cmd)
{
	std::string result;
	FILE* pipe_p;
	if (!(pipe_p = _popen(cmd.c_str(), "rt")))
	{
		return std::string("FAIL TO EXECUTE COMMAND");
	}
	char buffer_ps[8192];
	while (fgets(buffer_ps, 8192, pipe_p))
	{
		result += buffer_ps;
	}
	sprintf(buffer_ps, "\nProcess returned %d\n", _pclose(pipe_p));
	result += buffer_ps;
	return std::move(result);
}

// =============================================================================

void pulse_proc(TCPCLS::BlockPack& args_pack)
{
	send_responce(RCMD_SUCCESS);
}

void execute_cmd_proc(TCPCLS::BlockPack& args_pack)
{
	std::unique_ptr<char> cmd_line(new char[args_pack.size()]);
	args_pack.extract(cmd_line.get(), args_pack.size());
	send_responce(RCMD_SUCCESS);
	std::string output = exe_cmd(cmd_line.get());
	MemPack output_pack(output.c_str(), output.size() + 1);
	g_to_server_p->send(output_pack);
	end_check();
}

void send_file_proc(TCPCLS::BlockPack& args_pack)
{
	std::unique_ptr<char> dsc_path(new char[args_pack.size()]);
	args_pack.extract(dsc_path.get(), args_pack.size());
	auto file_deleter = [](FILE* p) {fclose(p); };
	std::unique_ptr<FILE, decltype(file_deleter)> file_ptr(fopen(dsc_path.get(), "wb"), file_deleter);
	if (!file_ptr)
	{
		send_responce(RCMD_ARGS_ERROR);
		return;
	}
	send_responce(RCMD_SUCCESS);
	RecvPack& recv_pack = g_to_server_p->recv();
	char buffer_p[8192];
	while (size_t n = recv_pack.extract(buffer_p, 8192))
	{
		fwrite(buffer_p, 1, n, file_ptr.get());
	}
	end_check();
}

void recv_file_proc(TCPCLS::BlockPack& args_pack)
{
	std::unique_ptr<char> src_path(new char[args_pack.size()]);
	args_pack.extract(src_path.get(), args_pack.size());
	try
	{
		FilePack file_pack(src_path.get());
		send_responce(RCMD_SUCCESS);
		g_to_server_p->send(file_pack);
		end_check();
	}
	catch (FileError& e)
	{
		send_responce(RCMD_ARGS_ERROR);
	}
}
