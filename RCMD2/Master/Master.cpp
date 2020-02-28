#include "Master.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "cmd_functions.h"

DEFAULT_CONSOLE_BEGIN(g_console)
REG_DEFAULT_HELP()
REG_DEFAULT_EXIT()
REG_DEFAULT_CLEAR()
REG_CMD(send_file, "sf", "send file to slave machine.")
REG_CMD(recv_file, "rf", "recieve file from slave machine.")
REG_CMD(execute_cmd, "cmd", "execute command on slave machine.")
REG_CMD(connect_server, "connect", "connect (ip) (port) | connect to server.")
REG_CMD(list_slaves, "ls", "list slaves to seize control.")
REG_CMD(seize_control, "sc", "sc (i) | seize control of slave with index i.")
REG_CMD(show_status, "status", "show present status.")
DEFAULT_CONSOLE_END()

bool init()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return false;
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return false;
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return false;
	return true;
}

int main()
{
	if (init())
		return g_console.main();
}
