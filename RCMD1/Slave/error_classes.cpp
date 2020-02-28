#include "error_classes.h"
#include <iostream>
#include "Slave.h"

void error_proc(TCPCLS::ERR::WSAError& e)
{
	std::clog << "fatal error occurs (WSAError:" << e._code << ")" << std::endl;
	delete g_to_server_p;
	g_to_server_p = nullptr;
	std::clog << "disconnected" << std::endl;
}
