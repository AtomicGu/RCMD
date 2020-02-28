#include "class_MasterListener.h"
#include <unistd.h>
#include "Server.h"
#include "class_MasterAttendant.h"

// ===== loggings =====
namespace mloggins
{
	void starting_logging()
	{
		g_console.lock();
		g_console << ANSI_TBLUE << ANSI_LIGHT << "[Master Listener] starting.\n";
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}

	void started_logging()
	{
		g_console.lock();
		g_console << ANSI_TGREEN << ANSI_LIGHT << "[Master Listener] started.\n";
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}

	void connected_logging(const std::string& ip, int index)
	{
		g_status_board.lock();
		auto& mi = g_status_board._master_info[index];
		mi._ip = ip;
		g_status_board.unlock();
		g_console.lock();
		g_console << ANSI_TGREEN << ANSI_LIGHT << "[Master Listener] master connected: ip=" << ip << '\n';
		g_console << ANSI_DEFAULT << std::flush;
		g_console.unlock();
	}
}

using namespace mloggins;

//==============================================================================

MasterListener::MasterListener(MasterPool& pool, unsigned short port, int backlog) :
	_pool(pool),
	_port(port),
	_backlog(backlog),
	_t()
{}

void MasterListener::start()
{
	_t = std::thread(&MasterListener::main, this);
	_t.detach();
}

void assign_attendant(MasterPool& pool, int index)
{
	if (g_attendant_pa[index])
		delete g_attendant_pa[index];
	g_attendant_pa[index] = new MasterAttendant(&pool, index, &g_slave_pool);
}

void MasterListener::main()
{
	starting_logging();
	eztcp::Listener ml(_port, _backlog);
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
		Master* s_p = new Master(ml.accept());
		locker_ptr->replace(s_p);
		connected_logging(s_p->get_ip(), locker_ptr.index());
		assign_attendant(_pool, locker_ptr.index());
	}
}
