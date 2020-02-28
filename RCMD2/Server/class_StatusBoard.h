#pragma once
#include <string>
#include <map>
#include <mutex>

struct MasterInfo
{
	int _slave_index{ -1 };
	std::string _ip;
};

struct SlaveInfo
{
	int _master_index{ -1 };
	std::string _ip;
};

class StatusBoard :public std::mutex
{
public:
	std::map<int, MasterInfo> _master_info;
	std::map<int, SlaveInfo> _slave_info;
};
