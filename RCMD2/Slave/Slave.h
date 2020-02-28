#pragma once

#ifndef _DEBUG
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#ifdef DEBUG
constexpr const char* g_server_ip_ps = "127.0.0.1"; // 服务器ip地址
#else
constexpr const char* g_server_ip_ps = "47.101.35.106"; // 服务器ip地址
#endif

constexpr unsigned short g_server_port = 23334; // 服务器端口号
