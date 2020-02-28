#pragma once
#include "../lib/TCPCLSWIN.h"

void send_responce(int result_code);

void pulse_proc(TCPCLS::BlockPack& args_pack);

void execute_cmd_proc(TCPCLS::BlockPack& args_pack);

void send_file_proc(TCPCLS::BlockPack& args_pack);

void recv_file_proc(TCPCLS::BlockPack& args_pack);
