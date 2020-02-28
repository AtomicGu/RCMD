// filereg: server
//TODO:主线程、副线程所在地
//主线程负责监听host连接请求，连接host对象
//副线程负责监听guest连接请求，创建guest对象
//第二步：3rd线程负责管理连接

#include "pch.h"
#include <thread>//TODO:用非阻塞模式改写
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

//==========全局变量==========
host *host_list[2] = { 0 };
guest *guest_list[16] = { 0 };
char host_num = 0;
char guest_num = 0;
char EM_rotor_file_path[] = R"(E:\RDEC\MyWorks\VS\C++\filereg\Release\ROTORSTD)";

HANDLE MY_HANDLE;
//==========全局变量==========

char* COLOR(TEXT_COLOR color)
{
	SetConsoleTextAttribute(MY_HANDLE, color);
	static char a[] = "";
	return a;
}

int vice()
{
	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6459);
	addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
	bind(listener, (sockaddr*)&addr, sizeof(struct sockaddr));
	sockaddr_in c_addr;
	int sockaddrlen = sizeof(struct sockaddr);
	SOCKET ac;
	char password[] = "123456789";
	while (true)
	{
		listen(listener, 3);
		ac = accept(listener, (sockaddr*)&c_addr, &sockaddrlen);
		int index = 0;
		while (guest_list[index]) ++index;
		guest_list[index] = new guest(ac, password);
	}
	return 0;
}

int main()
{
	MY_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);

	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	std::thread vice_thread(vice);
	vice_thread.detach();

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6469);
	addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
	bind(listener, (sockaddr*)&addr, sizeof(struct sockaddr));
	sockaddr_in c_addr;
	int sockaddrlen = sizeof(struct sockaddr);
	SOCKET ac;
	while (true)
	{
		listen(listener, 3);
		ac = accept(listener, (sockaddr*)&c_addr, &sockaddrlen);
		int index = 0;
		while (host_list[index]) ++index;
		host_list[index] = new host(ac);
	}
}
