#pragma once
// command codes
#define RCMD_PULSE 0
#define RCMD_CHECK_END 1
#define RCMD_EXECUTE_CMD 2
#define RCMD_SEND_FILE 3
#define RCMD_RECV_FILE 4
#define RCMD_GET_SLAVE_LIST 5
#define RCMD_SEIZE_CONTROL 6

// result codes
#define RCMD_SUCCESS 0
#define RCMD_ORDER_ERROR -1
#define RCMD_ARGS_ERROR -2
#define RCMD_SLAVE_LOST_ERROR -3
#define RCMD_SLAVE_BUSY_ERROR -4
#define RCMD_NO_SLAVE_ERROR -5
