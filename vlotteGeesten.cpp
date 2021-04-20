#include "Arduino.h"
#include "ArduinoJson.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>

class VlotteGeesten {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
  const int ledPin = 23; // the number of the neopixel strip
  const int numLeds = 64;
  int LDR_VALUE__ONE;
  int LDR_VALUE__TWO;
  int LDR_VALUE__THREE;
  int LDR_VALUE__FOUR;
  int LDR_VALUE__FIVE;
  String game;
  int rondeNummer;
  int id;
  
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
      Serial.println("Vlotte geesten!");

      socket->on("rondeNummer", [&](const char* payload, size_t len) {
      DynamicJsonDocument doc(512);
      auto error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
      }
      rondeNummer = doc["rondeNummer"].as<int>();
      Serial.println("rondeNummer");
      Serial.println(rondeNummer);
      });

  } 
   int analogReadAvg(int pin) {
      int vals = 0;
      for (int i = 0; i < 5; i++) {
      vals+=analogRead(pin);
      }
      return vals/5;
    }

    void loop() {
      Serial.println(LDR_VALUE__ONE);
      
      LDR_VALUE__ONE = analogReadAvg(26);
      LDR_VALUE__TWO = analogReadAvg(25);
      LDR_VALUE__THREE = analogReadAvg(34);
      LDR_VALUE__FOUR = analogReadAvg(35);
      LDR_VALUE__FIVE = analogReadAvg(39);

      if(LDR_VALUE__ONE < 500){
        socket->emit("objecten", String("{ \"object\": \"spook\", \"game\": \"vlottegeest\", \"id\": " + String(gameId) + ", \"rondeNummer\": " + String(rondeNummer) + " }").c_str());
        strip->setPixelColor(19, 0, 0, 255);
        Serial.print(LDR_VALUE__ONE);
      }else{
        strip->setPixelColor(19, 0, 0,0);
      }
      

      if(LDR_VALUE__TWO < 500){
        socket->emit("objecten", String("{ \"object\": \"bad\", \"game\": \"vlottegeest\", \"id\": " + String(gameId) + ", \"rondeNummer\": " + String(rondeNummer) + " }").c_str());
        strip->setPixelColor(26, 255, 0,0);
      
      }else{
        strip->setPixelColor(26, 0, 0,0);
      }

      if(LDR_VALUE__THREE < 500){
        socket->emit("objecten", String("{ \"object\": \"frogie\", \"game\": \"vlottegeest\", \"id\": " + String(gameId) + ", \"rondeNummer\": " + String(rondeNummer) + " }").c_str());
        strip->setPixelColor(33, 0, 128, 0);
      
      }else{
        strip->setPixelColor(33, 0, 0,0);
      }

      if(LDR_VALUE__FOUR < 500){
        socket->emit("objecten", String("{ \"object\": \"dokie\", \"game\": \"vlottegeest\", \"id\": " + String(gameId) + ", \"rondeNummer\": " + String(rondeNummer) + " }").c_str());
        strip->setPixelColor(42, 138, 186,211);
      
      }else{
        strip->setPixelColor(42, 0, 0,0);
      }

      if(LDR_VALUE__FIVE < 500){
        socket->emit("objecten", String("{ \"object\": \"borstel\", \"game\": \"vlottegeest\", \"id\": " + String(gameId) + ", \"rondeNummer\": " + String(rondeNummer) + " }").c_str());
        strip->setPixelColor(51, 255, 255,255);
      
      }else{
        strip->setPixelColor(51, 0, 0,0);
      }
      
      strip->show();
    }
};
