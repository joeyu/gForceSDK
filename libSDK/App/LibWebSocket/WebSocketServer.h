#ifndef _WEB_SOCKET_SERVER_H_
#define _WEB_SOCKET_SERVER_H_

#include <Windows.h>
#include <string>
#include <map>

class TCPSocketServer;
class WebSocketHandler;

class WebSocketServer
{
public:
	WebSocketServer() : m_thread(NULL) {}
	virtual ~WebSocketServer() { Deinit(); }

	// Initializes 
	int Init(int port);
	void Deinit();

	// Starts to accept connections in a separate thread, and then stops it in desire
	int Run();
	void Stop();

	// Path handler
	void SetWebSocketHandler(const char* path, WebSocketHandler* handler);
	WebSocketHandler* GetWebSocketHandler(const char* path);
	
	// Gets the underlying TCP-based socket server 
	TCPSocketServer* GetTCPSocketServer() { return &m_tcpSocketServer; }

private:
	typedef std::map<std::string, WebSocketHandler*> WebSocketHandlerContainer;

	TCPSocketServer m_tcpSocketServer;
	WebSocketHandlerContainer m_webSocketHandlerContainter;

	// Thread to accept connections 
	HANDLE m_thread;

	// Thread entry function
	static unsigned ThreadEntry(void *arg);
};

#endif
