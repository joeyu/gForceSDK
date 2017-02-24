#ifndef TCPSOCKETSERVER_H
#define TCPSOCKETSERVER_H

#include "Socket.h"
//#include "TCPSocketConnection.h"

class TCPSocketConnection;

class TCPSocketServer : public Socket
{
public:
	TCPSocketServer() : m_isListening(false) {}

	int Bind(int port);

	int Listen(int backlog = 1);
	bool IsListening() { return m_isListening; }

	// Only accepts 1 connection per call
	TCPSocketConnection* Accept();

private:
	bool m_isListening;
};

#endif
