#include "Arduino.h"
#include "SocketIoClient.h"

class TrivialPursuit {
private:
	SocketIoClient* socket;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setup() {
		Serial.println("Trivial pursuit!!");
	}

	void loop() {

	}
};