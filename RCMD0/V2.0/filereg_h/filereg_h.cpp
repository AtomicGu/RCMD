//#define ROTORSTD_PATH "ROTORSTD"
#define ROTORSTD_PATH R"(E:\RDEC\MyWorks\VS\C++\filereg32\Debug\ROTORSTD)"
#define DEFAULT_KEY "123456789"

#include <iostream>
#include "H2S_Connection.h"
#include <string>
#include <sstream>

H2S_Connection *TO = NULL;
std::istringstream *args;

//CONNECT
int CONNECT()
{
	char server_ip[16];
	unsigned short port = 6469;
	*args >> server_ip;
	if(TO) delete TO;
	TO = new H2S_Connection(server_ip, port, ROTORSTD_PATH, DEFAULT_KEY, DEFAULT_KEY);
	return TO->state();
}

//lg
int list_guests()
{
	TO->s_order('\1', NULL, 0);
	char *back;
	int size;
	TO->s_feedback(back, size);
	for (int i = 0; i < size; ++i)
	{
		printf("%s", &back[i]);
	}
	return 0;
}

//sc
int seize_control()
{
	int a;
	std::cin >> a;
	TO->s_order('\2', (const char*)&a, 4);
	char *back;
	int size;
	TO->s_feedback(back, size);
	if (*back) throw *back;
	TO->reset_g_coder(ROTORSTD_PATH, DEFAULT_KEY);
	return 0;
}

//info
int info()
{
	TO->s_order('\3', NULL, 0);
	char *back;
	int size;
	TO->s_feedback(back, size);
	for (int i = 0; i < size; ++i)
	{
		printf("%s", &back[i]);
	}
	return 0;
}

//tf
int transport_file()
{
	//initialize
	char host_path[256], guest_path[256];
	if (!(*args >> host_path)) throw 1;
	if (!(*args >> guest_path)) throw 1;
	FILE *f;
	if ((f = fopen(host_path, "rb")) == NULL) throw 2;
	
	//send order pack
	char order = 4;
	TcpPacket_EM order_pack(&order, 1);
	TO->g_send(&order_pack);

	//send name pack
	TcpPacket_EM name_pack(guest_path, 256);
	TO->g_send(&name_pack);

	//send data pack
	TcpPacket_EM file_pack(f);
	TO->g_send(&file_pack);

	//return
	fclose(f);
	return 0;
}

//tf
int transport_folder(char *folder_path, char *target_path)
{
	//TODO:trans folder
	return 0;
}

//sys
int sys()
{
	//initialize
	char cmd[256];
	*args >> cmd;
	
	//send order pack
	char order = 5;
	TcpPacket_EM order_pack(&order, 1);
	TO->g_send(&order_pack);

	//send cmd pack
	TcpPacket_EM cmd_pack((char*)cmd, 256);
	TO->g_send(&cmd_pack);

	//recv re pack
	TcpPacket_EM *re_pack;
	TO->g_recv(re_pack);

	//print re pack
	int size = re_pack->size();
	char *re = new char[re_pack->size()];
	re_pack->extract(re, -1);
	std::cout << re << std::endl;
	delete re;
	delete re_pack;
	return 0;
}

void debug(int r)
{
	struct exception
	{
		int sn;
		char text[24];
	};
	exception list[] = {
		1,"参数不足",
		2,"文件打开失败",

		-1,"收发错误",
		-2,"未收到预期答复",
		-3,"收到空包",
	};

	int num = sizeof(list) / sizeof(*list);
	for (int i = 0; i < num; ++i)
	{
		if (r == list[i].sn)
		{
			std::cerr << "explain: " << list[i].text << std::endl;
			break;
		}
	}
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
		debug(r);
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

	char help[][24] = {
		"shutdown",
		"CONNECT - use first",
		"lg - list guest",
		"sc - seize control",
		"info - get info",
		"tf - transport file",
		"tf - transport_folder",
		"sys - send cmd command",
		"help - get help",
	};

	char cmd_list[][8] = {
		"CONNECT",
		"lg",
		"sc",
		"info",
		"tf",
		"sys",
	};

	int(*func_list[])() = {
		CONNECT,
		list_guests,
		seize_control,
		info,
		transport_file,
		sys,
	};

	std::string input;
	args = new std::istringstream(input);
	while (true)
	{
		try
		{
			std::cout << '>';
			getline(std::cin, input);
			delete args;
			args = new std::istringstream(input);

			//==========命令行处理==========
			std::string cmd;
			*args >> cmd;
			int num = sizeof(func_list) / sizeof(*func_list);
			int i;
			for (i = 0; i < num; ++i)
			{
				if (cmd == cmd_list[i])
				{
					ans(func_list[i]());
					break;
				}
			}
			if (i == num)
			{
				if (cmd == "shutdown")
				{
					break;
				}
				else if (cmd == "help")
				{
					for (i = 0; i < num; ++i)
					{
						printf("%s\n", help[i]);
					}
				}
				else
				{
					std::cout << "Command invalid" << std::endl;
				}
			}
			//==========命令行处理==========
		}
		catch (int error)
		{
			ans(error);
			delete TO;
			std::cerr << "Connection breaked" << std::endl;
		}
	}
	WSACleanup();
	return 0;
}
