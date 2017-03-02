#pragma once
#include <Windows.h>
#include <WinHttp.h>

class WebSocket {
public:
	WebSocket() : m_session(NULL), m_connection(NULL), m_webSocket(NULL) {}
	int Open(const WCHAR *server, INTERNET_PORT port);
	void Close();
	int Send(WINHTTP_WEB_SOCKET_BUFFER_TYPE buf_type, void *buf, size_t size);
	int Receive(void *buf, size_t size, size_t *received_size, WINHTTP_WEB_SOCKET_BUFFER_TYPE *buf_type);
private:
	HINTERNET	m_session;
	HINTERNET	m_connection;
	HINTERNET	m_webSocket;
	static const WCHAR * const m_userAgent;
	static const WCHAR * const m_path;
};