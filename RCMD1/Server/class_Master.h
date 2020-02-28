#pragma once
#include <thread>
#include <mutex>
#include <memory>
#include "../lib/TCPCLSWIN.h"
#include "class_slave.h"

class Slave;

void slave_deleter(Slave* p);

typedef std::unique_ptr<Slave, void(*)(Slave*)> UPSlave;

class Master
{
public:
	Master(TCPCLS::TCPSession s);

public:
	bool alive();	// ͨ�����std::mutex _alive�ж��Ƿ����

private:
	TCPCLS::TCPSession _s;
	std::mutex _alive;
	UPSlave _my_slave;
	std::thread _thread;

private:
	void thread_main();
	void pulse_proc(TCPCLS::BlockPack& args_pack);
	void get_slave_list_proc(TCPCLS::BlockPack& args_pack);
	void seize_control_proc(TCPCLS::BlockPack& args_pack);
	void execute_cmd_proc(TCPCLS::BlockPack& args_pack);
	void send_file_proc(TCPCLS::BlockPack& args_pack);
	void recv_file_proc(TCPCLS::BlockPack& args_pack);

};
