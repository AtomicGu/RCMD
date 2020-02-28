#pragma once
#include <thread>
#include "class_SlavePool.h"

class SlaveListener
{
public:
	SlaveListener(SlavePool& pool, unsigned short port, int backlog);

public:
	void start();

private:
	SlavePool& _pool;
	unsigned short _port;
	int _backlog;
	std::thread _t;

private:
	void main();
};

/* ���ԸĽ��ĵط���
   ������ʱ���������⣬��Ϊֻ�ǶԺ�̨�ػ��̵߳ļ򵥷�װ��
   �����ʱֻ���Ǵ��̼߳�����У�һֱ�������˳�����ʱ�����������Ѿ�����ν */
