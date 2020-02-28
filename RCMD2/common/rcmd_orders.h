#pragma once
#include "eztcp/eztcp.h"

// command codes
#define RCMD_PULSE 0
#define RCMD_EXECUTE_CMD 1
#define RCMD_SEND_FILE 2
#define RCMD_RECV_FILE 3
#define RCMD_GET_SLAVE_LIST 4
#define RCMD_SEIZE_CONTROL 5

// feedback codes
#define RCMD_SUCCESS 0
#define RCMD_ORDER_ERROR -1
#define RCMD_ARGS_ERROR -2
#define RCMD_SLAVE_LOST_ERROR -3
#define RCMD_SLAVE_BUSY_ERROR -4
#define RCMD_NO_SLAVE_ERROR -5

void send_order(eztcp::Session & s, INT32 command_code, eztcp::MemPack & arg_pack);

INT32 recv_order(eztcp::Session& s, eztcp::BlockPack& arg_pack);

void send_feedback(eztcp::Session& s, INT32 feedback_code);

INT32 recv_feedback(eztcp::Session& s);

struct CodeError
{
	int _code;
	CodeError(int code) : _code(code) {}
	int what() { return _code; };
};

struct FeedbackError :public CodeError
{
	FeedbackError(int code) :CodeError(code) {}
};

struct OrderError :public CodeError
{
	OrderError(int code) :CodeError(code) {}
};
