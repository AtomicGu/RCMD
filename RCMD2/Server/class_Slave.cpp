#include "class_Slave.h"
#include <utility>
#include "Server.h"

// ===== loggings =====

void construct_slave_logging()
{
	g_console.lock();
	g_console << ANSI_TBLUE << ANSI_LIGHT << "[i] new slave\n";
	g_console << ANSI_DEFAULT << std::flush;
	g_console.unlock();
}

void destruct_slave_logging(std::string& ip)
{
	g_console.lock();
	g_console << ANSI_TBLUE << ANSI_LIGHT << "[Slave] disconnected: ip=" << ip << '\n';
	g_console << ANSI_DEFAULT << std::flush;
	g_console.unlock();
}

//==============================================================================

Slave::Slave(eztcp::Session&& s) :
	_port(s.get_port()),
	_ip(s.get_ip()),
	_s(std::move(s))
{
	//construct_slave_logging();	// not needed
}

Slave::~Slave()
{
	destruct_slave_logging(_ip);
}
