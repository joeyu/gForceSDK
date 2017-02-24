#ifndef _WEB_SOCKET_CONNECTION_H_
#define _WEB_SOCKET_CONNECTION_H_

#include "TCPSocketServer.h"
#include "WebSocketHandler.h"
class WebSocketServer;
class WebSocketConnection;


class WebSocketConnection
{
public:
	WebSocketConnection(TCPSocketConnection *tcpSocketConnection, WebSocketServer* server) :
		m_webSocketServer(server),
		m_tcpSocketConnection(tcpSocketConnection),
		m_prevFin(false),
		m_thread(NULL) {}
	virtual ~WebSocketConnection() {}

	int			Run();
	void			Stop();

	TCPSocketConnection*	GetTCPSocketConnection() { return m_tcpSocketConnection; }
	WebSocketServer*	GetWebSocketServer() { return m_webSocketServer; }

	static bool HandleHTTP(WebSocketConnection *webSocketConnection, char* buf, int size);
	static bool HandleWebSocket(WebSocketConnection *webSocketConnection, char* buf, int size);
	static bool SendUpgradeResponse(WebSocketConnection *webSocketConnection, char* key);

private:
	WebSocketConnection() : m_webSocketServer(NULL), m_tcpSocketConnection(NULL), m_prevFin(false), m_thread(NULL) {}

	WebSocketServer		*m_webSocketServer;
	TCPSocketConnection	*m_tcpSocketConnection;
	WebSocketHandler	*m_webSocketHandler;

	bool			m_prevFin;

	// Thread to accept connections 
	HANDLE			m_thread;

	// Thread entry function
	static unsigned ThreadEntry(void *arg);
};

#endif
