#include "Arduino.h"
#include "SocketIoClient.h"

class VierOpEenRij {
private:
	SocketIoClient* socket;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}

	void setup() {
		Serial.println("Vier op een rij!");
		socket->on("message", [](const char* payload, size_t len) {
			Serial.println("VoeR msg");
			Serial.println(payload);
		});
		socket->emit("example", "dit is een test");
		socket->emit("example", "dit is een test2");
		socket->emit("example", "dit is een test3");
		socket->emit("example", "{ \"test\": 2 }");
	}

	void loop() {

	}
};