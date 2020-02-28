#include "class_Master.h"
#include <utility>
#include "Server.h"

// ===== loggings =====

namespace mloggins
{
	void construct_master_logging()
	{
		g_console.lock();
		g_console << ANSI_TBLUE << ANSI_LIGHT << "[i] new master\n";
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}

	void destruct_master_logging(std::string& ip)
	{
		g_console.lock();
		g_console << ANSI_TBLUE << ANSI_LIGHT << "[Master] disconnected: ip=" << ip << '\n';
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}
}

using namespace mloggins;

//==============================================================================

Master::Master(eztcp::Session&& s) :
	_port(s.get_port()),
	_ip(s.get_ip()),
	_s(std::move(s))
{
	//construct_master_logging();
}

Master::~Master()
{
	destruct_master_logging(_ip);
}
