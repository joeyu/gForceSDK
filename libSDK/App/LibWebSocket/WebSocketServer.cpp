#include <process.h>

#include "WebSocketServer.h"
#include "WebSocketConnection.h"


int WebSocketServer::Init(int port)
{
	// Initialize WinSock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return -1;
	}


	return 0;
}

void WebSocketServer::Deinit()
{
	Stop();
	WSACleanup();
}

int WebSocketServer::Run()
{
	if (NULL != m_thread) {// running
		return 0;
	}

	if (!m_tcpSocketServer.Bind(port)) {
		return -1;
	}
	if (!m_tcpSocketServer.Listen()) {
		return -1;
	}

	// Create a thread in which start to accept connections immediately
	m_thread = (HANDLE)_beginthreadex(
		NULL,
		0,
		ThreadEntry,
		this,
		0,
		0,
		NULL);
	if (NULL == m_thread) {
		return -1;
	}
}

void WebSocketServer::Stop()
{
	if (NULL == m_thread) { // already stopped
		return;
	}

	// FIXME: synchronize before calling CloseHandle.
	CloseHandle(m_thread);
	m_thread = NULL;

	m_tcpSocketServer.Close();
}

void WebSocketServer::SetWebSocketHandler(const char* path, WebSocketHandler* handler)
{
	m_webSocketHandlerContainter[path] = handler;
}

WebSocketHandler* WebSocketServer::GetWebSocketHandler(const char* path)
{
	WebSocketHandlerContainer::iterator it;

	it = m_webSocketHandlerContainter.find(path);
	if (it != m_webSocketHandlerContainter.end()) {
		return it->second;
	}
	return NULL;
}

TCPSocketServer* WebSocketServer::GetTCPSocketServer()
{ 
	return &m_tcpSocketServer; 
}

unsigned WebSocketServer::ThreadEntry(void *arg)
{
	WebSocketServer *webSocketServer = (WebSocketServer*)arg;


	while (true) {
		// printf("accepting\r\n");
		TCPSocketServer *tcpSocketServer = webSocketServer->GetTCPSocketServer();
		TCPSocketConnection *tcpSocketConnection = tcpSocketServer->Accept();
		if (NULL == tcpSocketConnection) {
			continue;
		}
		WebSocketConnection *webSocketConnection = new WebSocketConnection(tcpSocketConnection, webSocketServer);
		if (!webSocketConnection->Run()) {
			printf("[ERROR] WebSocketConnection::Run");
			return -1;
		}
		// FIXME: Save the new connection
	}

	return 0;
}
