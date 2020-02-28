#include "cmdprocs.h"
#include <string>
#include <memory>
#include "rcmd_orders.h"

bool check_file(const char* file_path_ps, const char* mode_ps)
{
	FILE* f_p;
	if (!(f_p = fopen(file_path_ps, mode_ps)))
		return false;
	fclose(f_p);
	return true;
}

std::string execute_cmd(const std::string& cmd)
{
	std::string result;
	FILE* pipe_p;
	if (!(pipe_p = _popen(cmd.c_str(), "rt")))
		return std::string("FAIL TO EXECUTE COMMAND");
	char buffer_ps[8192];
	while (fgets(buffer_ps, 8192, pipe_p))
		result += buffer_ps;
	sprintf(buffer_ps, "\nProcess returned %d\n", _pclose(pipe_p));
	result += buffer_ps;
	fclose(pipe_p);
	return std::move(result);
}

//==============================================================================

void pulse_proc(eztcp::Session& s, eztcp::BlockPack& args_pack)
{
	send_feedback(s, RCMD_SUCCESS);
}

void execute_cmd_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack)
{
	std::string result = execute_cmd(arg_pack.get_buf());
	send_feedback(s, RCMD_SUCCESS);
	eztcp::MemPack result_pack(result.c_str(), result.size() + 1);
	s << result_pack;
}

void send_file_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack)
{
	if (!check_file(arg_pack.get_buf(), "wb"))
	{
		send_feedback(s, RCMD_ARGS_ERROR);
	}
	send_feedback(s, RCMD_SUCCESS);
	eztcp::FilePack recv_file_pack(s.recv(), arg_pack.get_buf());
}

void recv_file_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack)
{
	if (!check_file(arg_pack.get_buf(), "rb"))
	{
		send_feedback(s, RCMD_ARGS_ERROR);
	}
	send_feedback(s, RCMD_SUCCESS);
	eztcp::FilePack send_file_pack(arg_pack.get_buf());
	s << send_file_pack;
}
