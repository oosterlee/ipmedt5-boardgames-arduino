#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>

class TrivialPursuit {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
  int positions[20] = {9, 10, 11, 12, 13, 14, 17, 30, 33, 46, 49, 50, 51, 52, 53, 54, 41, 38, 25, 22};
  int players [4] = {0, 0, 0, 0};
  const int p1 = 13;
  const int p2 = 12;
  const int p3 = 33;
  const int p4 = 32;

public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
		Serial.println("Trivial pursuit!!");
		strip->begin();
		strip->setBrightness(240); // 1/3 brightness
	}

	void loop() {
  websocket
  if(touchRead(p1) < 25){
    strip->setPixelColor(positions[players[0]], 0,0,0);
    players[0] = players[0] + 1;
    strip->setPixelColor(positions[players[0]], 50,0,0);
  }
  else if(touchRead(p2) < 25){
    strip->setPixelColor(positions[players[1]], 0,0,0);
    players[1] = players[1] + 1;
    strip->setPixelColor(positions[players[1]], 0,0,50);
  }
  else if(touchRead(p3) < 25){
    strip->setPixelColor(positions[players[2]], 0,0,0);
    players[2] = players[2] + 1;
    strip->setPixelColor(positions[players[2]], 0,50,0);
  }
  else if(touchRead(p4) < 25){
    strip->setPixelColor(positions[players[3]], 0,0,0);
    players[3] = players[3] + 1;
    strip->setPixelColor(positions[players[3]], 50,50,0);
  }
  strip->show();
  delay(1000);
	}
};
