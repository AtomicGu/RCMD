#pragma once
#include <string>
#include <eztcp/eztcp.h>
#include "class_Slave.h"

class Slave;

class Master
{
public:
	Master(eztcp::Session&& s);
	Master(const Master& cpy) = delete;
	~Master();

public:
	Master& operator=(const Master& cpy) = delete;

public:
	eztcp::Session& get_session() { return _s; };
	unsigned short get_port() { return _port; }
	std::string get_ip() { return _ip; }

private:
	unsigned short _port;
	std::string _ip;
	eztcp::Session _s;
};
