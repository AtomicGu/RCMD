#pragma once

/* for create a global CmdMap */
#define CMDMAP_BEGIN(name) stlcli::CmdMap name {
#define CMDMAP_END() };

/* for create a global Console */
#define DEFAULT_CONSOLE_BEGIN(name) stlcli::Console name (stlcli::CmdMap{
#define DEFAULT_CONSOLE_END() });

/* register commands, use between BEGIN and END */
#define REG_CMD(cmd_proc,token,remarks) {(token),stlcli::Cmd{(cmd_proc),(remarks)}},
#define REG_DEFAULT_HELP() {"help",stlcli::Cmd{stlcli::default_cmd_help_proc,"show all avaliable commands."}},
#define REG_DEFAULT_CLEAR() {"clear",stlcli::Cmd{stlcli::default_cmd_clear_proc,"clear the console."}},
#define REG_DEFAULT_EXIT() {"exit",stlcli::Cmd{stlcli::default_cmd_exit_proc,"exit program with a num used as a exit code."}},

/* define process function using default arguments */
#define DEFINE_PROC_CMD(name) void name(std::istream& argi, stlcli::Console& con)
#define DEFINE_PROC_WRONGCMD(name) void name(const std::string& cmd, stlcli::Console& con)
#define DEFINE_PROC_BLANKCMD(name) void name(stlcli::Console& con)
#define DEFINE_PROC_ARGUMENTERROR(name) void name(stlcli::err::ArgumentError& e, stlcli::Console& con)

/* ANSI effects control */
#define ANSI_DEFAULT	"\033[0m"
#define ANSI_LIGHT		"\033[1m"
#define ANSI_LINE		"\033[4m"
#define ANSI_SHINE		"\033[5m"
#define ANSI_INVERT		"\033[7m"
#define ANSI_HIDE		"\033[8m"

/* ANSI text color control */
#define ANSI_TBLACK		"\033[30m"
#define ANSI_TRED		"\033[31m"
#define ANSI_TGREEN		"\033[32m"
#define ANSI_TYELLOW	"\033[33m"
#define ANSI_TBLUE		"\033[34m"
#define ANSI_TPURPLE	"\033[35m"
#define ANSI_TDGREEN	"\033[36m"
#define ANSI_TWHITE		"\033[37m"

/* ANSI background color control */
#define ANSI_BBLACK		"\033[40m"
#define ANSI_BRED		"\033[41m"
#define ANSI_BGREEN		"\033[42m"
#define ANSI_BYELLOW	"\033[43m"
#define ANSI_BBLUE		"\033[44m"
#define ANSI_BPURPLE	"\033[45m"
#define ANSI_BDGREEN	"\033[46m"
#define ANSI_BWHITE		"\033[47m"
