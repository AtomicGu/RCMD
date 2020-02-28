#pragma once

#include <thread>
#include "EMSTD.h"
#include "TcpPacket_EM.h"

class guest;

class host
{
private:
	EMSTD::MACHINE *_s_coder;
public:
	guest *linkto;
	SOCKET _mysocket;

	host();
	~host();
	int execute();
	int feedback();
	int transmit(int lenth);
	int waiting();
};
