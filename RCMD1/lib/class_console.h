#ifdef CLASSCONSOLE_EXPORTS
#define CLASSCONSOLE_API __declspec(dllexport)
#else
#define CLASSCONSOLE_API __declspec(dllimport)
#endif

#define CMD_MAP_BEGIN(cmd_map) CmdMap cmd_map = {
#define CMD_MAP(cmd) {cmd._token,cmd},
#define CMD_MAP_END() };

#include <map>
#include <iostream>
#include <sstream>
#include <string>

class Console;
void CLASSCONSOLE_API default_help_proc(Console* p_csl, std::istream& args_in);
void CLASSCONSOLE_API default_exit_proc(Console* p_csl, std::istream& args_in);
void CLASSCONSOLE_API default_wrong_cmd_proc(Console* p_csl, const std::string& cmd);
void CLASSCONSOLE_API default_blank_cmd_proc(Console* p_csl);

typedef void (*pCmdFunc)(Console* p_csl, std::istream& args_in);

typedef void (*pWrongCmdProc)(Console* p_csl, const std::string& cmd);

typedef void (*pBlankCmdProc)(Console* p_csl);

struct CmdInfo
{
	pCmdFunc _function;
	std::string _token;
	std::string _discription;
};

typedef std::map<std::string, CmdInfo> CmdMap;

class CLASSCONSOLE_API Console
{
public:
	CmdMap _cmd_map;
	pWrongCmdProc _p_wrong_cmd_proc;
	pBlankCmdProc _p_blank_cmd_proc;
	std::istream& _in;
	std::ostream& _out;

	Console(
		const CmdMap& cmd_map,
		pWrongCmdProc p_wrong_cmd_proc = default_wrong_cmd_proc,
		pBlankCmdProc p_blank_cmd_proc = default_blank_cmd_proc,
		std::istream& in = std::cin,
		std::ostream& out = std::cout
	);
	Console(
		CmdMap&& cmd_map,
		pWrongCmdProc p_wrong_cmd_proc = default_wrong_cmd_proc,
		pBlankCmdProc p_blank_cmd_proc = default_blank_cmd_proc,
		std::istream& in = std::cin,
		std::ostream& out = std::cout
	);
	Console(const Console&) = delete;
	Console(Console&&) = delete;
	int main();
	void end(int main_return);
};

extern CmdInfo CLASSCONSOLE_API default_help;
extern CmdInfo CLASSCONSOLE_API default_exit;
