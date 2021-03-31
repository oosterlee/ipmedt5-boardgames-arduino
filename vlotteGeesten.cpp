#include "Arduino.h"
#include "SocketIoClient.h"

class VlotteGeesten {
private:
	SocketIoClient* socket;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setup() {
		Serial.println("Vlotte geesten!");
	}

	void loop() {

	}
};