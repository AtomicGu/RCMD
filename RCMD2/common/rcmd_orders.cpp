#include "rcmd_orders.h"

class CodePack :public eztcp::BlockPack
{
public:
	CodePack(INT32 order_code) :eztcp::BlockPack(&order_code, sizeof(order_code)) {}
	CodePack(eztcp::RecvPack& recv) :eztcp::BlockPack(recv) {}

public:
	int code() { return *(this->get_buf()); }
};

void send_order(eztcp::Session& s, INT32 command_code, eztcp::MemPack& arg_pack)
{
	eztcp::MemPack code_pack(&command_code, sizeof(command_code));
	s.send(code_pack);
	s.send(arg_pack);
}

INT32 recv_order(eztcp::Session& s, eztcp::BlockPack& arg_pack)
{
	CodePack code_pack = s.recv();
	arg_pack = s.recv();
	return code_pack.code();
}

void send_feedback(eztcp::Session& s, INT32 feedback_code)
{
	eztcp::MemPack code_pack(&feedback_code, sizeof(feedback_code));
	s.send(code_pack);
}

INT32 recv_feedback(eztcp::Session& s)
{
	CodePack code_pack = s.recv();
	return code_pack.code();
}
