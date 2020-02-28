#include "tcp_client.h"

tcp_client::tcp_client(const char *ip, int port)
{
	reconnect(ip, port);
}

tcp_client::~tcp_client()
{
	closesocket(_mysocket);
}

int tcp_client::reconnect(const char *ip, int port)
{
	_mysocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.S_un.S_addr = inet_addr(ip);
	connected = connect(_mysocket, (sockaddr*)&server, sizeof(server));
	return connected;
}

int tcp_client::tcp_send(char* buf,int len)
{
	return send(_mysocket, buf, len, 0);
}

int tcp_client::tcp_recv(char* buf, int len)
{
	return recv(_mysocket, buf, len, 0);
}
