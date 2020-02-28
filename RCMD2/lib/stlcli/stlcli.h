#pragma once
#include <iostream>
#include <mutex>
#include <map>
#include <stdexcept>

namespace stlcli
{
	namespace err
	{
		/* argument error types */
		typedef std::invalid_argument ArgumentError;
	}

	/* forehead statement */
	class Console;

	/* default proccess functions */
	void default_wrong_cmd_proc(const std::string& cmd, Console& con);
	void default_blank_cmd_proc(Console& con);
	void default_argument_error_proc(err::ArgumentError& e, Console& con);
	void default_cmd_help_proc(std::istream& argi, Console& con);
	void default_cmd_exit_proc(std::istream& argi, Console& con);
	void default_cmd_clear_proc(std::istream& argi, Console& con);

	/* console used function types */
	typedef void (*pfCmdProc)(std::istream& argi, Console& con);
	typedef void (*pfWrongCmdProc)(const std::string& cmd, Console& con);
	typedef void (*pfBlankCmdProc)(Console& con);
	typedef void (*pfArgumentErrorProc)(err::ArgumentError& e, Console& con);

	/* command type */
	struct Cmd
	{
		pfCmdProc _cmd_proc;
		std::string _remarks;
	};

	/* command map type */
	typedef std::map<std::string, Cmd> CmdMap;

	/* class console */
	class Console :public std::ostream, public std::mutex
	{
	public:
		pfWrongCmdProc _wrong_cmd_proc_pf;
		pfBlankCmdProc _blank_cmd_proc_pf;
		pfArgumentErrorProc _argument_error_proc_pf;
		std::istream& _in;
		CmdMap _cmd_map;

	public:
		Console(
			const CmdMap& cmd_map,
			std::istream& in = std::cin,
			std::ostream& out = std::cout,
			pfWrongCmdProc wrong_cmd_proc_pf = default_wrong_cmd_proc,
			pfBlankCmdProc blank_cmd_proc_pf = default_blank_cmd_proc,
			pfArgumentErrorProc argument_error_proc_pf = default_argument_error_proc
		);

		Console(
			CmdMap&& cmd_map,
			std::istream& in = std::cin,
			std::ostream& out = std::cout,
			pfWrongCmdProc wrong_cmd_proc_pf = default_wrong_cmd_proc,
			pfBlankCmdProc blank_cmd_proc_pf = default_blank_cmd_proc,
			pfArgumentErrorProc argument_error_proc_pf = default_argument_error_proc
		);

	public:
		int main();
		void end(int main_return);
	};
}
