#include <string>
#include <map>

#include "WebSocketConnection.h"
#include "WebSocketServer.h"
#include "TCPSocketConnection.h"
#include "sha1.h"

#define UPGRADE_WEBSOCKET	"Upgrade: websocket"
#define SEC_WEBSOCKET_KEY	"Sec-WebSocket-Key:"
#define MAGIC_NUMBER		"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define OP_CONT		0x0
#define OP_TEXT		0x1
#define OP_BINARY	0x2
#define OP_CLOSE	0x8
#define OP_PING		0x9
#define OP_PONG		0xA

int WebSocketConnection::Run()
{
	// Create a thread in which start to accept connections immediately
	m_thread = (HANDLE)_beginthreadex(
		NULL,
		0,
		ThreadEntry,
		this,
		0,
		0,
		NULL);
	if (0 == m_thread) {
		return -1;
	}

	return 0;
}

void WebSocketConnection::Stop() 
{
	// FIXME: synchronize before calling CloseHandle.
	CloseHandle(m_thread);
	m_thread = NULL;

	// printf("Closed\r\n");
	m_tcpSocketConnection->Close();
}

bool WebSocketConnection::HandleHTTP(WebSocketConnection *webSocketConnection, char* buf, int size)
{
	char* line = &buf[0];
	char key[128];
	bool isUpgradeWebSocket = false;
	bool isSecWebSocketKeyFound = false;

	WebSocketServer *webSocketServer = webSocketConnection->GetWebSocketServer();

	for (int i = 0; i < size; i++) {
		if (buf[i] == '\r' && i+1 < size && buf[i+1] == '\n') {
			buf[i] = '\0';
			if (strlen(buf) <= 0) {
				break;
			}
			//printf("[%s]\r\n", line);
			if (line == &buf[0]) {
				char* method = strtok(buf, " ");
				char* path = strtok(NULL, " ");
				char* version = strtok(NULL, " ");
				// printf("[%s] [%s] [%s]\r\n", method, path, version);
				webSocketConnection->m_webSocketHandler = webSocketServer->GetWebSocketHandler(path);
				if (!webSocketConnection->m_webSocketHandler) {
					printf("ERROR: Handler not found for %s\r\n", path);
					return false;
				}
			} else if (strnicmp(line, UPGRADE_WEBSOCKET, strlen(UPGRADE_WEBSOCKET)) == 0) {
				isUpgradeWebSocket = true;
			} else if (strnicmp(line, SEC_WEBSOCKET_KEY, strlen(SEC_WEBSOCKET_KEY)) == 0) {
				isSecWebSocketKeyFound = true;
				char* ptr = line + strlen(SEC_WEBSOCKET_KEY);
				while (*ptr == ' ') ++ptr;
				strcpy(key, ptr);
			}
			i += 2;
			line = &buf[i];
		}
	}

	if (isUpgradeWebSocket && isSecWebSocketKeyFound) {
		this->sendUpgradeResponse(key);
		if (m_webSocketHandler) {
			m_webSocketHandler->onOpen();
		}
		m_prevFin = true;
		return true;
	}

	return false;
}

bool WebSocketConnection::HandleWebSocket(WebSocketConnection *webSocketConnection, char* buf, int size)
{
	uint8_t* ptr = (uint8_t*)buf;

	bool fin = (*ptr & 0x80) == 0x80;
	uint8_t opcode = *ptr & 0xF;

	if (opcode == OP_PING) {
		*ptr = ((*ptr & 0xF0) | OP_PONG);
		m_tcpSocketConnection.send_all(buf, size);
		return true;
	}
	if (opcode == OP_CLOSE) {
		if (m_webSocketHandler) {
			m_webSocketHandler->onClose();
		}
		return false;
	}
	ptr++;

	if (!fin || !m_prevFin) {	
		printf("WARN: Data consists of multiple frame not supported\r\n");
		m_prevFin = fin;
		return true; // not an error, just discard it
	}
	m_prevFin = fin;

	bool mask = (*ptr & 0x80) == 0x80;
	uint8_t len = *ptr & 0x7F;
	ptr++;
	
	if (len > 125) {
		printf("WARN: Extended payload length not supported\r\n");
		return true; // not an error, just discard it
	}

	char* data;
	if (mask) {
		char* maskingKey = (char*)ptr;
		data = (char*)(ptr + 4);
		for (int i = 0; i < len; i++) {
        	data[i] = data[i] ^ maskingKey[(i % 4)];
        }
	} else {
		data = (char*)ptr;
	}
	if (m_webSocketHandler) {
		if (opcode == OP_TEXT) {
			data[len] = '\0';
			m_webSocketHandler->onMessage(data);
		} else if (opcode == OP_BINARY) {
			m_webSocketHandler->onMessage(data, len);
		}
	}
	return true;
}

char* base64Encode(const uint8_t* data, size_t size,
                   char* outputBuffer, size_t outputBufferSize)
{
	static char encodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	                               'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	                               'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	                               'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	                               'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	                               'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	                               'w', 'x', 'y', 'z', '0', '1', '2', '3',
	                               '4', '5', '6', '7', '8', '9', '+', '/'};
    size_t outputLength = 4 * ((size + 2) / 3);
    if (outputBufferSize - 1 < outputLength) { // -1 for NUL
    	return NULL;
    }

    for (size_t i = 0, j = 0; i < size; /* nothing */) {
        uint32_t octet1 = i < size ? (unsigned char)data[i++] : 0;
        uint32_t octet2 = i < size ? (unsigned char)data[i++] : 0;
        uint32_t octet3 = i < size ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet1 << 0x10) + (octet2 << 0x08) + octet3;

        outputBuffer[j++] = encodingTable[(triple >> 3 * 6) & 0x3F];
        outputBuffer[j++] = encodingTable[(triple >> 2 * 6) & 0x3F];
        outputBuffer[j++] = encodingTable[(triple >> 1 * 6) & 0x3F];
        outputBuffer[j++] = encodingTable[(triple >> 0 * 6) & 0x3F];
    }

	static int padTable[] = { 0, 2, 1 };
	int paddingCount = padTable[size % 3];

    for (int i = 0; i < paddingCount; i++) {
        outputBuffer[outputLength - 1 - i] = '=';
    }
    outputBuffer[outputLength] = '\0'; // NUL

    return outputBuffer;
}

bool WebSocketConnection::SendUpgradeResponse(WebSocketConnection *webSocketConnection, char* key)
{
	char buf[128];

	if (strlen(key) + sizeof(MAGIC_NUMBER) > sizeof(buf)) {
		return false;
	}
	strcpy(buf, key);
	strcat(buf, MAGIC_NUMBER);

    uint8_t hash[20];
	SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, (unsigned char*)buf, strlen(buf));
    SHA1Result(&sha, (uint8_t*)hash);

	char encoded[30];
    base64Encode(hash, 20, encoded, sizeof(encoded));

    char resp[] = "HTTP/1.1 101 Switching Protocols\r\n" \
	    "Upgrade: websocket\r\n" \
    	"Connection: Upgrade\r\n" \
    	"Sec-WebSocket-Accept: XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n\r\n";
    char* ptr = strstr(resp, "XXXXX");
    strcpy(ptr, encoded);
    strcpy(ptr+strlen(encoded), "\r\n\r\n");

    int ret = m_tcpSocketConnection.send_all(resp, strlen(resp));
    if (ret < 0) {
    	printf("ERROR: Failed to send response\r\n");
    	return false;
    }

    return true;
}

unsigned WebSocketServer::ThreadEntry(void *arg)
{
	WebSocketConnection *webSocketConnection = (WebSocketConnection*)arg;

	char buf[1024];
	bool isWebSocket = false;

	TCPSocketConnection *tcpSocketConnection = webSocketConnection->GetTCPSocketConnection();
	while (tcpSocketConnection->IsValid()) {
		int ret = tcpSocketConnection->Receive(buf, sizeof(buf) - 1);
		if (ret > 0) {
			if (!isWebSocket) {
				if (WebSocketConnection::HandleHTTP(webSocketConnection, buf, ret)) {
					isWebSocket = true;
				}
				else {
					printf("ERROR: Non websocket\r\n");
					break;
				}
			}
			else {
				if (!WebSocketConnection::HandleWebSocket(webSocketConnection, buf, ret)) {
					break;
				}
			}
		}
		else if (ret == 0) {
			// Shut down
			return 0;
		}
		else if (ret < 0) {
			printf("ERROR: Failed to receive %d\r\n", ret);
			return ret;
		}
	}
}