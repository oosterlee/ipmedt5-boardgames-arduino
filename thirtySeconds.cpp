#include "Arduino.h"
#include "SocketIoClient.h"

class ThirtySeconds {
private:
	SocketIoClient* socket;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setup() {
		Serial.println("30 seconds!");
	}

	void loop() {

	}
};