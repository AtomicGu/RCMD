#include "class_SlaveListener.h"
#include <unistd.h>
#include "Server.h"

// ===== loggings =====
namespace sloggings
{
	void starting_logging()
	{
		g_console.lock();
		g_console << ANSI_TBLUE << ANSI_LIGHT << "[Slave Listener] starting.\n";
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}

	void started_logging()
	{
		g_console.lock();
		g_console << ANSI_TGREEN << ANSI_LIGHT << "[Slave Listener] started.\n";
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}

	void connected_logging(const std::string& ip, int index)
	{
		g_status_board.lock();
		auto& si = g_status_board._slave_info[index];
		si._ip = ip;
		g_status_board.unlock();
		g_console.lock();
		g_console << ANSI_TGREEN << ANSI_LIGHT << "[Slave Listener] slave connected: ip=" << ip << '\n';
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}
}

using namespace sloggings;

//==============================================================================

SlaveListener::SlaveListener(SlavePool& pool, unsigned short port, int backlog) :
	_pool(pool),
	_port(port),
	_backlog(backlog),
	_t()
{}

void SlaveListener::start()
{
	_t = std::thread(&SlaveListener::main, this);
	_t.detach();
}

void SlaveListener::main()
{
	starting_logging();
	eztcp::Listener sl(_port, _backlog);
	started_logging();
	while (true)
	{
		auto locker_ptr = _pool.rent_vacancy();
		if (!locker_ptr)
		{
			_pool.refresh_and_clean();
			if (!(locker_ptr = _pool.rent_vacancy()))
			{
				sleep(300);
				continue;
			}
		}
		Slave* s_p = new Slave(sl.accept());
		connected_logging(s_p->get_ip(), locker_ptr.index());
		locker_ptr->replace(s_p);
	}
}
