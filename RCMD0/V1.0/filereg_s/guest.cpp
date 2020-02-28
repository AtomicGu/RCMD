//guest对象是静态的，只是静态数据和相关函数的集合体
//不绑定线程，受host调用支配
//本质上来说，只是一个转发器
//

//HACK:感觉有大毛病

#include "pch.h"

extern host *host_list[2];
extern guest *guest_list[16];
extern char host_num;
extern char guest_num;
extern char EM_rotor_file_path[];
extern char* COLOR(TEXT_COLOR);

guest::guest(SOCKET S, char *password)
{
	self_socket = S;
	sockaddr_in c_addr;
	int c_addr_len = sizeof(c_addr);
	getsockname(self_socket, (sockaddr*)&c_addr, &c_addr_len);
	self_ip = inet_ntoa(c_addr.sin_addr);
	linkto = NULL;
	_coder = EMSTD::machine(EM_rotor_file_path, password);
	std::cout << COLOR(F_CYAN) << "CONNECT: guest - " << self_ip << std::endl << COLOR(F_WHITE);
}
//DONE

guest::~guest()
{
	closesocket(self_socket);
	if (linkto)
	{
		linkto->linkto = NULL;
	}
	int i = 0;
	while (guest_list[i] != this)
	{
		++i;
		if (i > 16) return;
	}
	host_list[i] = NULL;
	--host_num;
	EMSTD::free(_coder);
	std::cout << COLOR(F_CYAN) << "DISCONNECT: guest - " << self_ip << std::endl << COLOR(F_WHITE);
}
//DONE

int guest::_send_s(SOCKET S, char *buf, int len, int flags)
{
	for (int i = 0; i < len; ++i)
	{
		buf[i] = _coder->code(buf[i]);
	}
	return send(S, (const char*)buf, len, flags);
}
//DONE

int guest::_recv_s(SOCKET S, char *buf, int len, int flags)
{
	int ret = recv(S, buf, len, flags);
	for (int i = 0; i < len; ++i)
	{
		buf[i] = _coder->code(buf[i]);
	}
	return ret;
}
//DONE

int guest::transport_file()
{
	char cmd[] = "s4";
	_send_s(self_socket, cmd, 3, 0);

	char path[256];
	recv(linkto->self_socket , path, 256, 0);
	_send_s(self_socket, path, 256);

	int size = 0;
	recv(linkto->self_socket, (char*)&size, 4, 0);
	_send_s(self_socket, (char*)&size, 4);

	char buffer[8192];
	long i = 0;
	while (i < size)
	{
		i += recv(linkto->self_socket, buffer, 8192, 0);
		_send_s(self_socket, buffer, 8192);
	}
	char S_re;
	_recv_s(self_socket, &S_re, 1);
	send(linkto->self_socket, (const char*)&S_re, 1, 0);
	return 0;
}
//DONE

int guest::sys()
{
	//发送指令
	char cmd[] = "s5";
	_send_s(self_socket, cmd, 3, 0);
	//转发命令行
	char buff[1024];
	int n;
	for (n = 0; n < 1024; ++n)
	{
		recv(linkto->self_socket, &buff[n], 1, 0);
		if (!buff[n]) break;
	}
	_send_s(self_socket, buff, n, 0);
	//转发返回信息
	_recv_s(self_socket, buff, 1, 0);
	n = 0;
	while(buff[n])
	{
		for (n = 0; n < 1024; ++n)
		{
			recv(linkto->self_socket, &buff[n], 1, 0);
			if (!buff[n]) break;
		}
		send(linkto->self_socket, buff, n + 1, 0);
	}
	//转发结束标志
	char G_re;
	_recv_s(self_socket, &G_re, 1, 0);
	send(linkto->self_socket, &G_re, 1, 0);
	return 0;
}
//DONE

bool guest::pulse()
{
	if (send(self_socket, "", 0, 0) == SOCKET_ERROR) return false;
	else return true;
}
