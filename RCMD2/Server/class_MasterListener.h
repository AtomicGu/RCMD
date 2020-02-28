#pragma once
#include <thread>
#include "class_MasterPool.h"

class MasterListener
{
public:
	MasterListener(MasterPool& pool, unsigned short port, int backlog);

public:
	void start();

private:
	MasterPool& _pool;
	unsigned short _port;
	int _backlog;
	std::thread _t;

private:
	void main();
};

/* 可以改进的地方：（同SLT）
   析构的时候会出现问题，因为只是对后台守护线程的简单封装，
   在设计时只考虑此线程间断运行，一直到程序退出，此时析构的问题已经无所谓 */
