#pragma once
#include <WinSock2.h>
#include <thread>

class guest;

class host
{
private:
	static int _waiting(host *self);
public:
	SOCKET self_socket;
	const char *self_ip;
	std::thread *self_thread_p;
	guest *linkto;

	host(SOCKET S);
	~host();
	int h1();
	int h2();
	int h3();
	int h4();
	int h5();
	int waiting();

};