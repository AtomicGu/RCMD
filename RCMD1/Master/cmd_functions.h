#pragma once
#include "../lib/class_console.h"

void recv_file(Console* p_csl, std::istream& args_in);

void send_file(Console* p_csl, std::istream& args_in);

void execute_cmd(Console* p_csl, std::istream& args_in);

void seize_control(Console* p_csl, std::istream& args_in);

void list_slaves(Console* p_csl, std::istream& args_in);

void connect_server(Console* p_csl, std::istream& args_in);

void show_status(Console* p_csl, std::istream& args_in);
