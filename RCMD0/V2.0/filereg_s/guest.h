#pragma once

#include <WinSock2.h>
#include <thread>
#pragma comment(lib,"ws2_32.lib")

class host;

class guest
{
public:
	host *linkto;
	SOCKET _mysocket;
	std::thread *_mythread;

	guest(SOCKET mysocket);
	~guest();
	int transmit();
};
