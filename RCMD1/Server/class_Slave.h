#pragma once
#include <mutex>
#include "../lib/TCPCLSWIN.h"
#include "class_Master.h"

class Master;

class Slave
{
public:
	Slave(TCPCLS::TCPSession s);

public:
	void send_pack(TCPCLS::TCPPackage& pack) throw(TCPCLS::ERR::WSAError);
	TCPCLS::RecvPack& recv_pack() throw(TCPCLS::ERR::WSAError);
	bool try_own();
	void free();
	bool alive() noexcept;	// ͨ����Slave�����������ж��Ƿ���ţ������쳣����

private:
	TCPCLS::TCPSession _s;
	std::mutex _owned;
};
