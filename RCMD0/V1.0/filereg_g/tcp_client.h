#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

class tcp_client
{
protected:
	SOCKET _mysocket;
public:
	int connected;

	tcp_client(const char *ip, int port);
	~tcp_client();
	int reconnect(const char *ip, int port);
	int tcp_send(char* buf, int len);
	int tcp_recv(char* buf, int len);
};
