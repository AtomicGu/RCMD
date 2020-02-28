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

/* ���ԸĽ��ĵط�����ͬSLT��
   ������ʱ���������⣬��Ϊֻ�ǶԺ�̨�ػ��̵߳ļ򵥷�װ��
   �����ʱֻ���Ǵ��̼߳�����У�һֱ�������˳�����ʱ�����������Ѿ�����ν */
