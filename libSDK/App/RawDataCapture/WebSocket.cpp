// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include <Windows.h>
#include <WinHttp.h>
#include <stdio.h>

#include "WebSocket.h"

const WCHAR * const WebSocket::m_userAgent = L"OYMotion WebSocket";
const WCHAR * const WebSocket::m_path = L"/RawData";


int WebSocket::Open(const WCHAR *server, INTERNET_PORT port)
{
	DWORD dwError = ERROR_SUCCESS;
	BOOL fStatus = FALSE;
	HINTERNET hRequestHandle = NULL;
	HINTERNET hWebSocketHandle = NULL;


	//
	// Create session, connection and request handles.
	//

	m_session = WinHttpOpen(
		m_userAgent,
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		NULL,
		NULL,
		0);
	if (m_session == NULL)
	{
		dwError = GetLastError();
		goto quit;
	}

	m_connection = WinHttpConnect(m_session,
		server,
		port,
		0);
	if (m_connection == NULL)
	{
		dwError = GetLastError();
		goto quit;
	}

	hRequestHandle = WinHttpOpenRequest(m_connection,
		L"GET",
		m_path,
		NULL,
		NULL,
		NULL,
		0);
	if (hRequestHandle == NULL)
	{
		dwError = GetLastError();
		goto quit;
	}

	//
	// Request protocol upgrade from http to websocket.
	//
#pragma prefast(suppress:6387, "WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET does not take any arguments.")
	fStatus = WinHttpSetOption(hRequestHandle,
		WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
		NULL,
		0);
	if (!fStatus)
	{
		dwError = GetLastError();
		goto quit;
	}

	//
	// Perform websocket handshake by sending a request and receiving server's response.
	// Application may specify additional headers if needed.
	//

	fStatus = WinHttpSendRequest(hRequestHandle,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		NULL,
		0,
		0,
		0);
	if (!fStatus)
	{
		dwError = GetLastError();
		goto quit;
	}

	fStatus = WinHttpReceiveResponse(hRequestHandle, 0);
	if (!fStatus)
	{
		dwError = GetLastError();
		goto quit;
	}

	//
	// Application should check what is the HTTP status code returned by the server and behave accordingly.
	// WebSocketCompleteUpgrade will fail if the HTTP status code is different than 101.
	//

	m_webSocket = WinHttpWebSocketCompleteUpgrade(hRequestHandle, NULL);
	if (hWebSocketHandle == NULL)
	{
		dwError = GetLastError();
		goto quit;
	}

	//
	// The request handle is not needed anymore. From now on we will use the websocket handle.
	//

	WinHttpCloseHandle(hRequestHandle);
	hRequestHandle = NULL;

	wprintf(L"Succesfully upgraded to websocket protocol\n");

	return 0;
quit:
	if (hRequestHandle != NULL)
	{
		WinHttpCloseHandle(hRequestHandle);
		hRequestHandle = NULL;
	}
	Close();
	return -1;
}

    //
    // Send and receive data on the websocket protocol.
    //
int WebSocket::Send(WINHTTP_WEB_SOCKET_BUFFER_TYPE buf_type, void *buf, size_t size) {
	DWORD dwError = NO_ERROR;
	dwError = WinHttpWebSocketSend(m_webSocket,
		buf_type,
		buf,
		size);
	if (dwError != NO_ERROR)
	{
		return -1;
	}
	return 0;
}

    //wprintf(L"Sent message to the server: '%s'\n", pcwszMessage);

int WebSocket::Receive(void *buf, size_t size, size_t *received_size, WINHTTP_WEB_SOCKET_BUFFER_TYPE *buf_type) {
	DWORD dwError = NO_ERROR;

	dwError = WinHttpWebSocketReceive(m_webSocket,
		buf,
		size,
		(DWORD *)received_size,
		buf_type);
	if (dwError != ERROR_SUCCESS)
	{
		return -1;
	}

	//wprintf(L"Received message from the server: '%.*s'\n", dwBufferLength, (WCHAR*)rgbBuffer);
	return 0;
}


void WebSocket::Close(){
	DWORD dwError = NO_ERROR;

	if (m_webSocket != NULL)
	{
		//
		// Gracefully close the connection.
		//

		dwError = WinHttpWebSocketShutdown(m_webSocket,
			WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
			NULL,
			0);
		if (dwError != ERROR_SUCCESS)
		{
			printf("WinHttpWebSocketShutdown : %d", dwError);
		}

		dwError = WinHttpWebSocketClose(m_webSocket,
			WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
			NULL,
			0);
		if (dwError != ERROR_SUCCESS)
		{
			printf("WinHttpWebSocketClose: %d", dwError);
		}
		//
		// Check close status returned by the server.
		//

		USHORT usStatus = 0;
		BYTE rgbCloseReasonBuffer[123];
		DWORD dwCloseReasonLength = 0;
		dwError = WinHttpWebSocketQueryCloseStatus(m_webSocket,
			&usStatus,
			rgbCloseReasonBuffer,
			ARRAYSIZE(rgbCloseReasonBuffer),
			&dwCloseReasonLength);
		if (dwError != ERROR_SUCCESS)
		{
			printf("WinHttpWebSocketQueryCloseStatus: %d", dwError);
		}

		wprintf(L"The server closed the connection with status code: '%d' and reason: '%.*S'\n",
			(int)usStatus,
			dwCloseReasonLength,
			rgbCloseReasonBuffer);

		WinHttpCloseHandle(m_webSocket);
		m_webSocket = NULL;
	}

    if (m_connection != NULL)
    {
	    WinHttpCloseHandle(m_connection);
	    m_connection = NULL;
    }

    if (m_session != NULL)
    {
	    WinHttpCloseHandle(m_session);
	    m_session = NULL;
    }
}
