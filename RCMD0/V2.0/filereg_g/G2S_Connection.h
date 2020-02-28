#pragma once
#include "EMSTD.h"
#include "TcpPacket_EM.h"

class G2S_Connection
{
private:
	SOCKET _mysocket;
	int _state;

	EMSTD::MACHINE *_g_coder;

public:

	G2S_Connection(const char *ip, int port, const char *ROTORSTD, const char *g_key);
	~G2S_Connection();
	int state();

	int g_send(TcpPacket_EM *packet);
	int g_recv(TcpPacket_EM *&packet);

};
