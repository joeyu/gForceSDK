
#include <stdio.h>
#include <WinSock2.h>
#include "TCPSocketServer.h"



int TCPSocketServer::Bind(int port) {
	if (IsValid()) { // bound already
		return 0;
	}

	// Create the SOCKET
	m_sockFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_sockFd) {
		printf("[ERROR] ::socket\n");
		return -1;
	}

	if (0 != SetBlocking(true)) {
		printf("[ERROR] Socket::SetBlocking\n");
		return -1;
	}
	struct sockaddr_in server = { 0 };
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("localhost");
	server.sin_port = htons((u_short)port);

	// Bind the socket.
	if (::bind(m_sockFd, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR) 	{
		printf("[ERROR] ::bind.\n");
		return -1;
	}
	return 0;
}

int TCPSocketServer::Listen(int max) {
	if (!IsValid()) {
		return -1;
	}

	if (m_isListening == true) {
		return 0;
	}

	if (::listen(m_sockFd, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Error listening on socket.\n");
		return -1;
	}
	printf("Server is listening on socket... %s:%u\n");
	return 0;
}

TCPSocketConnection* TCPSocketServer::Accept() {
	if (!IsValid()) {
		return NULL;
	}

	struct sockaddr addr = { 0 };
	int len = sizeof(addr);
	SOCKET connect_fd = ::accept(m_sockFd, &addr, &len);
	if (INVALID_SOCKET == connect_fd) {
		printf("[Error] ::accept()")
		return NULL;
	}
	struct sockaddr_in* ipv4info = (struct sockaddr_in*)&addr;
	printf("[INFO] Connected from address %s:%d\n", inet_ntoa(ipv4info->sin_addr), ntohs(ipv4info->sin_port));

	if (0 != SetBlocking(true)) {
		printf("[ERROR] TCPSocketServer::SetBlocking\n");
		return -1;
	}
	TCPSocketConnection* connection = new TCPSocketConnection(connect_fd);
	if (0 != connection->SetBlocking(true)) {
		printf("[ERROR] Socket::SetBlocking\n");
		return -1;
	}
	connection->m_isConnected = true;

	return connection;
}