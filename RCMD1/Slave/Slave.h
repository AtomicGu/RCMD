#pragma once
#ifdef _DEBUG
#pragma comment(lib,"../Debug/TCPCLSWIN32.lib")
#else
#pragma comment(lib,"../Release/TCPCLSWIN32.lib")
#endif

#define HARMONY

#ifndef HARMONY
#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

#include "../lib/TCPCLSWIN.h"

#ifndef DEBUG
constexpr const char* g_server_ip_ps = "47.101.35.106"; // 服务器ip地址
#else
constexpr const char* g_server_ip_ps = "127.0.0.1"; // 服务器ip地址
#endif
constexpr unsigned short g_server_port = 23334; // 服务器端口号

extern TCPCLS::TCPSession* g_to_server_p;
