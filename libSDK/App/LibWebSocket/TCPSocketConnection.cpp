#include "TCPSocketConnection.h"


int TCPSocketConnection::Connect(const char* host, const int port)
{

    return 0;
}

int TCPSocketConnection::Send(char* data, int length)
{
	if (!IsValid()) {
		return -1;
	}
	return ::send(m_sockFd, data, length, 0);
}

int TCPSocketConnection::Receive(char* data, int length)
{
	if (!IsValid()) {
		return -1;
	}
	return ::recv(m_sockFd, data, length, 0);
}

