#ifndef _WEB_SOCKET_HANDLER_H_
#define _WEB_SOCKET_HANDLER_H_

class WebSocketHandler
{
public:
    virtual void onOpen() {};
    virtual void onClose() {};
    // to receive text message
    virtual void onMessage(char* text) {};
    // to receive binary message
    virtual void onMessage(char* data, size_t size) {};
    virtual void onError() {};
};

#endif
