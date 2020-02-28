#include <thread>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

//==========全局变量==========
host *host_list[2] = { 0 };
guest *guest_list[16] = { 0 };
char host_num = 0;
char guest_num = 0;
char EM_rotor_file_path[] = R"(E:\RDEC\MyWorks\VS\C++\filereg\Release\ROTORSTD)";
enum TEXT_COLOR { F_BLUE = 9, F_GREEN = 10, F_CYAN = 11, F_RED = 12, F_MEGENTA = 13, F_YELLOW = 14, F_WHITE = 15 };

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
