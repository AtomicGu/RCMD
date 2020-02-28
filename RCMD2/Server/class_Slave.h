#pragma once
#include <string>
#include <eztcp/eztcp.h>
#include "class_Master.h"

class Master;

class Slave
{
public:
	Slave(eztcp::Session&& s);
	~Slave();

public:
	eztcp::Session& get_session() { return _s; }
	unsigned short get_port() { return _port; }
	std::string get_ip() { return _ip; }

private:
	unsigned short _port;
	std::string _ip;
	eztcp::Session _s;
};
