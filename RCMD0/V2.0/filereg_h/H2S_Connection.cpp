#include "H2S_Connection.h"

H2S_Connection::H2S_Connection(const char *ip, int port, const char *ROTORSTD, const char *s_key, const char *g_key)
{
	_s_coder = EMSTD::machine(ROTORSTD, s_key);
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

H2S_Connection::~H2S_Connection()
{
	closesocket(_mysocket);
	EMSTD::free(_s_coder);
	EMSTD::free(_g_coder);
}

int H2S_Connection::s_order(char order, const char *args, int args_len)
{
	try
	{
		//send h2s head
		char head[5];
		*(int*)(head + 1) = args_len;
		for (int i = 0; i < 5; ++i)
		{
			head[i] = _s_coder->code(head[i]);
		}
		if (send(_mysocket, head, 5, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
		//send h2s body
		char *body = new char[args_len];
		for (int i = 0; i < args_len; ++i)
		{
			body[i] = _s_coder->code(args[i]);
		}
		if (send(_mysocket, body, args_len, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
		delete body;
		return 0;
	}
	catch (int error)
	{
		_state = WSAGetLastError();
		throw error;
	}
}

int H2S_Connection::s_feedback(char *&backs, int &size)
{
	try
	{
		//recv h2s head
		char head[5];
		if (recv(_mysocket, head, 5, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
		for (int i = 0; i < 5; ++i)
		{
			head[i] = _s_coder->code(head[i]);
		}
		size = *(int*)(head + 1);
		//recv h2s body
		static char *body = NULL;
		if (body) delete body;
		body = new char[size];
		if (recv(_mysocket, body, size, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
		for (int i = 0; i < size; ++i)
		{
			body[i] = _s_coder->code(body[i]);
		}
		backs = body;
		return 0;
	}
	catch (int error)
	{
		_state = WSAGetLastError();
		throw error;
	}
}

int H2S_Connection::state()
{
	return _state;
}

int H2S_Connection::g_send(TcpPacket_EM *packet)
{
	try
	{
		//send h2s head
		char head[5];
		head[0] = 0;
		*(int*)(head + 1) = packet->size() + 4;
		for (int i = 1; i < 5; ++i)
		{
			head[i] = _s_coder->code(head[i]);
		}
		if (send(_mysocket, head, 5, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
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

int H2S_Connection::g_recv(TcpPacket_EM *&packet)
{
	try
	{
		//recv h2s head
		char head[5];
		if (recv(_mysocket, head, 5, 0) == SOCKET_ERROR) throw SOCKET_ERROR;
		for (int i = 0; i < 5; ++i)
		{
			head[i] = _s_coder->code(head[i]);
		}
		if (head[0]) throw - 2;
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

void H2S_Connection::reset_g_coder(const char *ROTORSTD, const char *g_key)
{
	delete _g_coder;
	_g_coder = EMSTD::machine(ROTORSTD, g_key);
}