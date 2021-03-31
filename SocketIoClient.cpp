#include "SocketIoClient.h"


const String getEventName(const String msg) {
	return msg.substring(4, msg.indexOf("\"",4));
}

const String getEventPayload(const String msg) {
	// String result = msg.substring(msg.indexOf("\"",4)+2,msg.length()-1);
	// if ((char)result[0] == '"' && (char)result[result.length()-1] == '"') {
	// 	result = result.substring(1, msg.length()-3);
	// }
	// return result;
	String result = msg.substring(msg.indexOf("\"",4)+2,msg.length()-1);
	if(result.startsWith("\"")) {
		result.remove(0,1);
	}
	if(result.endsWith("\"")) {
		result.remove(result.length()-1);
	}
	return result;
}

static void hexdump(const uint32_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
#ifdef DEBUG_WEBSOCKETS
        Serial.printf("%08x ", *src);
#endif
        ++src;
        if ((i + 1) % 4 == 0) {
#ifdef DEBUG_WEBSOCKETS
            Serial.printf("\n");
#endif
        }
    }
}


void SocketIoClient::loop() {
	webSocket.loop();

	for(auto packet=packets.begin(); packet != packets.end();) {
		if(webSocket.sendTXT(*packet)) {
#ifdef DEBUG_WEBSOCKETS
			Serial.printf("[SIoC] packet \"%s\" emitted\n", packet->c_str());
#endif
			packet = packets.erase(packet);
		} else {
			++packet;
		}
	}

	if(millis() - lastPing > ping_interval) {
		webSocket.sendTXT("2");
		lastPing = millis();
	}
}

void SocketIoClient::begin(const char* host, int port, const char* url) {
#ifdef SOCKETIO_SSL
	webSocket.beginSSL(host + String(url), port);
#else
	webSocket.begin(host + String(url), port);
#endif
    webSocket.onEvent(std::bind(&SocketIoClient::wsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void SocketIoClient::begin(const char* host, int port) {
#ifdef SOCKETIO_SSL
	webSocket.beginSSL(host, port);
#else
	webSocket.begin(host, port);
#endif
    webSocket.onEvent(std::bind(&SocketIoClient::wsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void SocketIoClient::on(const char* event, std::function<void (const char * payload, size_t length)> func) {
	_events[event] = func;
}

void SocketIoClient::emit(const char* event, const char* data) {
	String msg = String("42[\"");
	msg += event;
	msg += "\"";
	if(event) {
		msg += ",";
		if (data[0] != (char)'{') {
			msg += "\"";
			msg += data;
			msg += "\"";
		} else {
			msg += data;
		}
	}
	msg += "]";
	packets.push_back(msg);
}

void SocketIoClient::trigger(const char* event, const char* data, size_t len) {
	auto e = _events.find(event);
	if(e != _events.end()) {
#ifdef DEBUG_WEBSOCKETS
		Serial.printf("[SIoC] trigger event %s\n", event);
#endif
		e->second(data, len);
	} else {
#ifdef DEBUG_WEBSOCKETS
		Serial.printf("[SIoC] event %s not found. %d events available\n", event, _events.size());
#endif
	}
}

void SocketIoClient::wsEvent(WStype_t type, uint8_t * payload, size_t length) {
    String msg;
	switch(type) {
		case WStype_DISCONNECTED:
#ifdef DEBUG_WEBSOCKETS
			Serial.printf("[SIoC] Disconnected!\n");
#endif
			break;
		case WStype_CONNECTED:
#ifdef DEBUG_WEBSOCKETS
			Serial.printf("[SIoC] Connected to url: %s\n",  payload);
#endif
			break;
		case WStype_TEXT:
#ifdef DEBUG_WEBSOCKETS
			Serial.printf("[SIoC] Received packet with payload: %s\n", payload);
#endif
			msg = String((char*)payload);
			if(msg.startsWith("42")) {
				trigger(getEventName(msg).c_str(), getEventPayload(msg).c_str(), getEventPayload(msg).length());
			} else if(msg.startsWith("2")) {
				webSocket.sendTXT("3");
			} else if(msg.startsWith("40")) {
				trigger("connect", NULL, 0);
			} else if(msg.startsWith("41")) {
				trigger("disconnect", NULL, 0);
			}
			break;
		case WStype_BIN:
#ifdef DEBUG_WEBSOCKETS
			Serial.printf("[SIoC] get binary length: %u\n", length);
#endif
			hexdump((uint32_t*) payload, length);
		break;
	}
}

void SocketIoClient::disconnect() {
	webSocket.disconnect();
	trigger("disconnect", NULL, 0);
}

void SocketIoClient::setAuthorization(const char * user, const char * password) {
    webSocket.setAuthorization(user, password);
}