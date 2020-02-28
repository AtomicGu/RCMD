#include "host.h"
#include "guest.h"

guest::guest(SOCKET mysocket)
{
	_mysocket = mysocket;
}

guest::~guest()
{
	closesocket(_mysocket);
}

int guest::transmit()
{
	char a;
	while (true)
	{
		if (recv(_mysocket, &a, 1, 0) == SOCKET_ERROR) break;
		if (send(linkto->_mysocket, &a, 1, 0) == SOCKET_ERROR) break;
	}
	return 0;
}
