#include "G2S_Connection.h"

G2S_Connection::G2S_Connection(const char *ip, int port, const char *ROTORSTD, const char *g_key)
{
	_g_coder = EMSTD::machine(ROTORSTD, g_key);
	_mysocket = socket(AF_INET, SOCK_STREAM, 0);
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = inet_addr(ip);
		_state = connect(_mysocket, (sockaddr*)&addr, sizeof(addr));
	}
}

G2S_Connection::~G2S_Connection()
{
	closesocket(_mysocket);
	EMSTD::free(_g_coder);
}

int G2S_Connection::g_send(TcpPacket_EM *packet)
{
	try
	{
		//send h2s body
		if (SendPacket(_mysocket, packet, 0, _g_coder) == SOCKET_ERROR) throw SOCKET_ERROR;
		return 0;
	}
	catch (int error)
	{
		_state = WSAGetLastError();
		throw error;
	}
}

int G2S_Connection::g_recv(TcpPacket_EM *&packet)
{
	try
	{
		//recv h2s body
		int re = RecvPacket(_mysocket, packet, 0, _g_coder);
		if (re == SOCKET_ERROR) throw SOCKET_ERROR;
		if (re == 0) throw - 3;
		return 0;
	}
	catch (int error)
	{
		_state = WSAGetLastError();
		throw error;
	}
}

int G2S_Connection::state()
{
	return _state;
}
