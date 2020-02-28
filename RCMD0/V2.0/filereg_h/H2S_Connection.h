#pragma once

#include "TcpPacket_EM.h"
#include "EMSTD.h"

class H2S_Connection
{
private:
	SOCKET _mysocket;
	int _state;

	EMSTD::MACHINE *_s_coder;
	EMSTD::MACHINE *_g_coder;

public:

	H2S_Connection(const char *ip, int port, const char *ROTORSTD, const char *s_key, const char *g_key);
	~H2S_Connection();
	int state();
	

	int s_order(char order, const char *args, int args_len);
	int s_feedback(char *&backs, int &size);
	int g_send(TcpPacket_EM *packet);
	int g_recv(TcpPacket_EM *&packet);

	void reset_g_coder(const char *ROTORSTD, const char *g_key);
};

//�쳣�б�
//-1���շ�����
//-2��ָ�ƥ��
//-3���յ��հ�