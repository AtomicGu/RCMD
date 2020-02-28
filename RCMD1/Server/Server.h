#pragma once
#ifdef _DEBUG
#pragma comment(lib,"../Debug/TCPCLSWIN32.lib")
#else
#pragma comment(lib,"../Release/TCPCLSWIN32.lib")
#endif

#include <iostream>
#include <string>
#include <mutex>

constexpr unsigned short master_port = 23333;
constexpr int master_backlog = 2;
constexpr unsigned short slave_port = 23334;
constexpr int slave_backlog = 5;
constexpr int max_master_number = 2;
constexpr int max_slave_number = 10;

class Master;
class Slave;

struct MasterInfo
{
	Master* p{ nullptr };
	unsigned short port;
	std::string ip;
	std::mutex lock;
};

struct SlaveInfo
{
	Slave* p{ nullptr };
	unsigned short port;
	std::string ip;
	std::mutex lock;
};

extern MasterInfo master_pool[max_master_number];
extern SlaveInfo slave_pool[max_slave_number];
