#pragma once
#include <eztcp/eztcp.h>

void pulse_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack);

void execute_cmd_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack);

void send_file_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack);

void recv_file_proc(eztcp::Session& s, eztcp::BlockPack& arg_pack);
