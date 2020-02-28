#pragma once
#include <WinSock2.h>
#include "EMSTD.h"
#pragma comment(lib,"EMSTD.lib")

class host;

class guest
{
private:
	EMSTD::MACHINE *_coder;
	int _send_s(SOCKET S, char *buf, int len, int flags = 0);
	int _recv_s(SOCKET S, char *buf, int len, int flags = 0);
	
public:
	SOCKET self_socket;
	const char *self_ip;
	host *linkto;

	guest(SOCKET, char *password);
	~guest();
	int transport_file();
	int sys();
	bool pulse();
};
