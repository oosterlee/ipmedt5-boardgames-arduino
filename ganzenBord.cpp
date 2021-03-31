#include "Arduino.h"
#include "SocketIoClient.h"

class GanzenBord {
private:
	SocketIoClient* socket;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setup() {
		Serial.println("Ganzenbord!");
	}

	void loop() {

	}
};