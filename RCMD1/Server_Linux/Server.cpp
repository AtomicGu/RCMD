#include "Server.h"
#include "server_manage.h"
#include <thread>

MasterInfo master_pool[max_master_number];
SlaveInfo slave_pool[max_slave_number];

void show_start_info()
{
	std::cout << "Port: Master=" << master_port << ", Slave=" << slave_port << std::endl;
	std::cout << "Backlog: Master=" << master_backlog << ", Slave=" << slave_backlog << std::endl;
	std::cout << "Max Connection: Master=" << max_master_number << ", Slave=" << max_slave_number << std::endl;
	std::cout << "server started" << std::endl;
}

int main()
{
	show_start_info();
	std::thread(master_waiter).detach();
	std::cout << "master listener started" << std::endl;
	std::thread(slave_waiter).detach();
	std::cout << "slave listener started" << std::endl;
	resource_manager();
}
