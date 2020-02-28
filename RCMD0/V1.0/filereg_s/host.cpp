//尝试将host对象与线程捆绑
//只负责接受host的指令，并处理server的任务，对guest执行的指令仅转发，不执行
//

#include "pch.h"

extern host *host_list[2];
extern guest *guest_list[16];
extern char host_num;
extern char guest_num;
extern char* COLOR(TEXT_COLOR);

host::host(SOCKET S)
{
	self_socket = S;
	sockaddr_in c_addr;
	int c_addr_len = sizeof(c_addr);
	getsockname(self_socket, (sockaddr*)&c_addr, &c_addr_len);
	self_ip = inet_ntoa(c_addr.sin_addr);
	linkto = NULL;
	self_thread_p = new std::thread(_waiting, this);
	self_thread_p->detach();

	std::cout << COLOR(F_MEGENTA) << "CONNECT: host - " << self_ip << std::endl << COLOR(F_WHITE);
}
//DONE?

host::~host()
{
	closesocket(self_socket);
	if (linkto)
	{
		linkto->linkto = NULL;
	}
	int i = 0;
	while (host_list[i] != this)
	{
		++i;
		if (i > 2) return;
	}
	host_list[i] = NULL;
	--host_num;

	std::cout << COLOR(F_MEGENTA) << "DISCONNECT: host - " << self_ip << std::endl << COLOR(F_WHITE);
}
//DONE?

int host::h1()
{
	char max_guest = 16;//guest数组长度
	send(self_socket, &max_guest, 1, 0);
	for (int i = 0; i < max_guest; ++i)
	{
		if (guest_list[i] == NULL)
		{
			send(self_socket, "NULL", 5, 0);
		}
		else
		{
			if (guest_list[i]->pulse())
			{
				int len = 0;
				while (guest_list[i]->self_ip[len]) ++len;
				send(self_socket, guest_list[i]->self_ip, len + 1, 0);
			}
			else
			{
				delete guest_list[i];
				guest_list[i] = NULL;
				send(self_socket, "NULL", 5, 0);
			}
		}
	}
	send(self_socket, "\0", 1, 0);
	return 0;
}
//DONE

int host::h2()
{
	char index;
	recv(self_socket, &index, 1, 0);
	--index;
	if (guest_list[index] == NULL)
	{
		send(self_socket, "\1", 1, 0);
		return 0;
	}//判断是否有效，无效返回
	if (guest_list[index]->linkto)
	{
		send(self_socket, "\2", 1, 0);
		return 0;
	}//判断是否占用，占用返回
	if (linkto)
	{
		linkto->linkto = NULL;
	}
	linkto = guest_list[index];
	linkto->linkto = this;
	send(self_socket, "\0", 1, 0);
	//send(self_socket, '\0', 1, 0);
	//TODO:会导致数据发送不出去，调查一下
	return 0;
}
//DONE

int host::h3()
{
	char buffer[1024];
	int csr = 0;//cursor
	//ip info
	strcpy(buffer, "IP: ");
	csr = 4;
	for (int i = 0; linkto->self_ip[i]; ++i, ++csr)
	{
		buffer[csr] = linkto->self_ip[i];
	}
	buffer[csr] = '\0';
	//TODO:其他信息
	//
	send(self_socket, buffer, 1024, 0);
	send(self_socket, "\0", 1, 0);
	return 0;
}
//DONE

int host::h4()
{
	if (linkto->pulse()) return linkto->transport_file();
	else
	{
		delete linkto;
		return 4;
	}
}
//DONE

int host::h5()
{
	if (linkto->pulse()) return linkto->sys();
	else
	{
		delete linkto;
		return 5;
	}
}
//DONE

int host::_waiting(host *self)
{
	return self->waiting();
}
//DONE

int host::waiting()
{
	char cmd[3];
	int last_return;
	while (recv(self_socket, cmd, 3, 0) != SOCKET_ERROR)//you bug
	{
		//if (linkto == 0)
		//{
		//	//TODO:
		//	std::cerr << "ERROR::linkto = 0, break now." << std::endl;
		//	break;
		//}
		if (!strcmp(cmd, "h1"))
		{
			last_return = h1();
		}
		else if (!strcmp(cmd, "h2"))
		{
			last_return = h2();
		}
		else if (!strcmp(cmd, "h3"))
		{
			last_return = h3();
		}
		else if (!strcmp(cmd, "h4"))
		{
			last_return = h4();
		}
		else if (!strcmp(cmd, "h5"))
		{
			last_return = h5();
		}
		else
		{
			break;
		}
		if (last_return)
		{
			std::cout << COLOR(F_RED) << "HOST " << self_ip << ": raised an error <" << last_return << ">." << std::endl << COLOR(F_WHITE);
			break;
		}
		std::cout << COLOR(F_YELLOW) << "HOST " << self_ip << ": " << cmd << std::endl << COLOR(F_WHITE);
	}
	delete this;
	return 0;
}
//DONE
