#pragma once
#include "E:/WAS/Works/D_Work/Remote_CMD/lib/TCPCLSWIN.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))

class OrderPack :public TCPCLS::TCPPackage
{
public:
	int _code;

	OrderPack(int order_code) :
		_code(order_code),
		_position(0)
	{}
	OrderPack(TCPCLS::RecvPack& recv) :
		_position(0)
	{
		recv.extract((char*)& _code, sizeof(_code));
	}

	virtual __int64 size() const
	{
		return sizeof(_code);
	}
	virtual size_t extract(char* p_buffer, size_t buffer_size)
	{
		size_t extract_size = min(buffer_size, sizeof(_code) - _position);
		memcpy(p_buffer, &_code, extract_size);
		_position += extract_size;
		return extract_size;
	}
	virtual __int64 position() const
	{
		return _position;
	}

private:
	int _position;
};

class ResultPack :public OrderPack
{
public:
	ResultPack(int order_code) :
		OrderPack(order_code)
	{}
	ResultPack(TCPCLS::RecvPack& recv) :
		OrderPack(recv)
	{}
	operator bool() const
	{
		return _code;
	}
};
