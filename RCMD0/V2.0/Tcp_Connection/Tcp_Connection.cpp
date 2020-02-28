#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

class Tcp_Connection
{
protected:
	SOCKET _mysocket;
public:
	int state;
	
	Tcp_Connection(SOCKET S);
	Tcp_Connection(const char *ip, int port);
	~Tcp_Connection();
	int reconnect(const char *ip, int port);
	int tcp_send(char* buf, int len);
	int tcp_recv(char* buf, int len);
};

Tcp_Connection::Tcp_Connection(SOCKET S)
{
	_mysocket = S;
}

Tcp_Connection::Tcp_Connection(const char *ip, int port)
{
	reconnect(ip, port);
}

Tcp_Connection::~Tcp_Connection()
{
	closesocket(_mysocket);
}

int Tcp_Connection::reconnect(const char *ip, int port)
{
	_mysocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_port = htons(port);
	target.sin_addr.S_un.S_addr = inet_addr(ip);
	state = connect(_mysocket, (sockaddr*)&target, sizeof(target));
	return state;
}

int Tcp_Connection::tcp_send(char* buf, int len)
{
	return send(_mysocket, buf, len, 0);
}

int Tcp_Connection::tcp_recv(char* buf, int len)
{
	return recv(_mysocket, buf, len, 0);
}
