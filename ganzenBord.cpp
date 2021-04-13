#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>

class GanzenBord {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
		Serial.println("Ganzenbord!");
	}

	void loop() {

	}
};