#include "class_Slave.h"
#include "RCMD_MACRO.h"
#include "RCMDPacks.h"
#include "TCPCLS.h"

using namespace TCPCLS;
using namespace TCPCLS::ERR;

// =============================================================================

Slave::Slave(TCPCLS::TCPSession s) :
	_s(std::move(s))
{}

void Slave::send_pack(TCPCLS::TCPPackage& pack) throw(TCPCLS::ERR::WSAError)
{
	_s.send(pack);
}

TCPCLS::RecvPack& Slave::recv_pack() throw(TCPCLS::ERR::WSAError)
{
	return _s.recv();
}

bool Slave::try_own()
{
	return _owned.try_lock();
}

void Slave::free()
{
	_owned.unlock();
}

bool Slave::alive()	noexcept
{
	try
	{
		OrderPack order_pack(RCMD_PULSE);
		BlockPack no_args;
		_s.send(order_pack);
		_s.send(no_args);
		ResultPack result_pack = _s.recv();
		if (result_pack._code)
		{
			return false;
		}
		return true;
	}
	catch (WSAError&)
	{
		return false;
	}
}
