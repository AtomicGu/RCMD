#include "host.h"
#include "guest.h"

int host::transmit(int lenth)
{
	int i = 0;
	char buffer[1024];
	do
	{
		recv(_mysocket, buffer, 1024, 0);
		i += send(linkto->_mysocket, buffer, 1024, 0);
	} while (i + 1024 < lenth);
	recv(_mysocket, buffer, lenth - i, 0);
	send(linkto->_mysocket, buffer, lenth - i, 0);
}

int waiting()
{

}
