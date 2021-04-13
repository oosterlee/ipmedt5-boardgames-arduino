#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>

class VlotteGeesten {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
      Serial.println("Vlotte geesten!");
  }

    void loop() {
      LDR_VALUE__ONE = analogRead(25);
      LDR_VALUE__TWO = analogRead(26);
      LDR_VALUE__THREE = analogRead(34);
      LDR_VALUE__FOUR = analogRead(35);
      LDR_VALUE__FIVE = analogRead(39);

      if(LDR_VALUE__ONE < 600){
        
        strip->setPixelColor(19, 0, 0, 255);
      
      }else{
        strip->setPixelColor(19, 0, 0,0);
      }

      if(LDR_VALUE__TWO < 600){

        strip->setPixelColor(26, 255, 0,0);
      
      }else{
        strip->setPixelColor(26, 0, 0,0);
      }

      if(LDR_VALUE__THREE < 600){
        
        strip->setPixelColor(33, 0, 128, 0);
      
      }else{
        strip->setPixelColor(33, 0, 0,0);
      }

      if(LDR_VALUE__FOUR < 600){

        strip->setPixelColor(42, 138, 186,211);
      
      }else{
        strip->setPixelColor(42, 0, 0,0);
      }

      if(LDR_VALUE__FIVE < 600){

        strip->setPixelColor(51, 255, 255,255);
      
      }else{
        strip->setPixelColor(51, 0, 0,0);
      }
      
      strip->show();
    }
  
};