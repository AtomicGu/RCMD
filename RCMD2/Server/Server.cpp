#include "Server.h"
#include <iomanip>
#include <stlcli/stlcliex.h>
#include "class_MasterAttendant.h"
#include "class_MasterListener.h"
#include "class_SlaveListener.h"

// ===== global variables =====
MasterPool g_master_pool(kMaxMasterNumber);
SlavePool g_slave_pool(kMaxSlaveNumber);
MasterAttendant* g_mas_p[kMaxMasterNumber] = { 0 };
MasterAttendant* g_attendant_pa[kMaxMasterNumber];
StatusBoard g_status_board;

//==============================================================================

void welcome()
{
	g_console.lock();
	g_console << "Port: Master=" << kMasterPort << ", Slave=" << kSlavePort << std::endl;
	g_console << "Backlog: Master=" << kMasterBacklog << ", Slave=" << kSlaveBacklog << std::endl;
	g_console << "Max Connection: Master=" << kMaxMasterNumber << ", Slave=" << kMaxSlaveNumber << std::endl;
	g_console << ANSI_TGREEN << ANSI_LIGHT << "=== server started ===" << ANSI_DEFAULT << std::endl;
	g_console.unlock();
}

int main()
{
	welcome();
	MasterListener ml(g_master_pool, kMasterPort, kMasterBacklog);
	ml.start();
	SlaveListener sl(g_slave_pool, kSlavePort, kSlaveBacklog);
	sl.start();
	return g_console.main();
}

DEFINE_PROC_CMD(kick_proc)
{
	int index;
	argi >> index;
	if (g_attendant_pa[index])
	{
		if (g_attendant_pa[index]->is_ended())
		{
			g_console.lock();
			g_console << ANSI_TGREEN << ANSI_LIGHT << "target is already ended." << ANSI_DEFAULT << std::endl;
			g_console.unlock();
		}
		else
		{
			g_attendant_pa[index]->send_stop_signal();
			g_console.lock();
			g_console << ANSI_TBLUE << ANSI_LIGHT << "signal sended, it may take a while for the target to stop." << ANSI_DEFAULT << std::endl;
			g_console.unlock();
		}
	}
	else
	{
		g_console.lock();
		g_console << ANSI_TRED << ANSI_LIGHT << "the target is invalid." << ANSI_DEFAULT << std::endl;
		g_console.unlock();
	}
}

DEFINE_PROC_CMD(masters_proc)
{
	g_status_board.lock();
	g_console.lock();
	g_console << ANSI_TPURPLE << ANSI_LIGHT << "Masters\n"
		<< "Index\tIP               Slave" << ANSI_DEFAULT << std::endl;
	for (int i = 0; i < kMaxMasterNumber; ++i)
	{
		auto& mi = g_status_board._master_info[i];
		g_console << std::right << std::setw(5) << i << '\t'
			<< std::left << std::setw(17) << mi._ip
			<< mi._slave_index << '(' << g_status_board._slave_info[i]._ip << ')' << std::endl;
	}
	g_console << std::right;
	g_console.unlock();
	g_status_board.unlock();
}

DEFINE_PROC_CMD(slaves_proc)
{
	g_status_board.lock();
	g_console.lock();
	g_console << ANSI_TPURPLE << ANSI_LIGHT << "Slaves\n"
		<< "Index\tIP" << ANSI_DEFAULT << std::endl;
	for (int i = 0; i < kMaxSlaveNumber; ++i)
	{
		auto si = g_status_board._slave_info[i];
		g_console << std::right << std::setw(5) << i << '\t'
			<< std::left << si._ip << std::endl;
	}
	g_console.unlock();
	g_status_board.unlock();
}

DEFINE_PROC_CMD(attendants_proc)
{
	g_console.lock();
	g_console << ANSI_TPURPLE << ANSI_LIGHT << "Attendants\n"
		<< "Index\tStatus" << ANSI_DEFAULT << std::endl;
	for (int i = 0; i < kMaxMasterNumber; ++i)
	{
		g_console << std::right << std::setw(5) << i << '\t';
		if (g_attendant_pa[i])
		{
			if (g_attendant_pa[i]->is_ended())
				g_console << "Stopped";
			else
				g_console << "Running";
		}
		else
		{
			g_console << "Vacant";
		}
		g_console << std::endl;
	}
	g_console.unlock();
}

DEFINE_PROC_CMD(clean_proc)
{
	int master_count = 0;
	for (int i = 0; i < kMaxMasterNumber; ++i)
	{
		if (auto ptr = g_master_pool.rent(i, false))
		{
			if (g_attendant_pa[ptr.index()] && g_attendant_pa[ptr.index()]->is_ended())
			{
				delete g_attendant_pa[ptr.index()];
				g_attendant_pa[ptr.index()] = nullptr;
				ptr->empty_out();
				g_status_board._master_info[ptr.index()] = MasterInfo();
				++master_count;
			}
		}
	}
	int slave_count = 0;
	for (int i = 0; i < kMaxSlaveNumber; ++i)
	{
		if (auto ptr = g_slave_pool.rent(i, false))
		{
			if (ptr->item_p() && !test_alive(*ptr->item_p()))
			{
				ptr->empty_out();
				g_status_board._slave_info[ptr.index()] = SlaveInfo();
				++slave_count;
			}
		}
	}
	g_console.lock();
	g_console << ANSI_TBLUE << ANSI_LIGHT << "clean " << master_count << " masters, " << slave_count << " slaves" << ANSI_DEFAULT << std::endl;
	g_console.unlock();
}

DEFAULT_CONSOLE_BEGIN(g_console)
REG_CMD(kick_proc, "kick", "send close signal to master")
REG_CMD(masters_proc, "masters", "show all masters")
REG_CMD(slaves_proc, "slaves", "show all slaves")
REG_CMD(attendants_proc, "attendants", "show all attendants")
REG_CMD(clean_proc, "clean", "clean server resources")
REG_DEFAULT_CLEAR()
REG_DEFAULT_EXIT()
REG_DEFAULT_HELP()
DEFAULT_CONSOLE_END()
