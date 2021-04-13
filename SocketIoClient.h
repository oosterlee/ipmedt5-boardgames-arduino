#ifndef SIOC_H
#define SIOC_H

#include <WebSocketsClient.h>
#include <map>
#include "config.h"

class SocketIoClient {
private:
	WebSocketsClient webSocket;
	void wsEvent(WStype_t type, uint8_t * payload, size_t length);
	void trigger(const char* event, const char* data, size_t len);
	std::vector<String> packets;
	std::map<String, std::function<void (const char * payload, size_t length)>> _events;

	int lastPing;
	int ping_interval = 10000;

public:
	void loop();
	void begin(const char* host, int port, const char* url);
	void begin(const char* host, int port);
	
	void on(const char* event, std::function<void (const char * payload, size_t length)> func);	
	void emit(const char* event, const char* data);

	void remove(const char* event);

	void setAuthorization(const char * user, const char * password);
	void disconnect();
};

#endif