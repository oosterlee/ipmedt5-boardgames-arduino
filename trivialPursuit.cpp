#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>
#include "ArduinoJson.h" 
#include <map>

class TrivialPursuit {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
	int user_id = -1;
  int players [4] = {0, 0, 0, 0};
  uint8_t playerSize = 0;
  int playerPositions[4][2];
  int plek;
  int old_1 = 0;
  int old_2 = 0;
  int old_3 = 0;
  int old_4 = 0;
  const int TOUCH_PIN = 15;


  
  std::map<String, int> steps = {
    {"stap_0", 9},
    {"stap_1", 10},
    {"stap_2", 11},
    {"stap_3", 12},
    {"stap_4", 13},
    {"stap_5", 14},
    {"stap_6", 17},
    {"stap_7", 30},
    {"stap_8", 33},

    {"stap_9", 46},
    {"stap_10", 49},
    {"stap_11", 50},
    {"stap_12", 51},
    {"stap_13", 52},
    {"stap_14", 53},
    {"stap_15", 54},
    {"stap_16", 41},

    {"stap_17", 38},
    {"stap_18", 25},
    {"stap_19", 22},
  };

public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
		strip->begin();
		strip->setBrightness(240); // 1/3 brightness

    socket->on("tp_turnCard", [&](const char* payload, size_t len){
      Serial.println(payload);
      DynamicJsonDocument doc(512);
      auto error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
      }
      
      for (int i = 0; i < playerSize; ++i) {
        if(players[i] == "id"){
          returb "id".as<int>());
        }
      }
      
    }

    socket->on("tp_getUsers", [&](const char* payload, size_t len) {
      Serial.println("__GETUSERS__");
      Serial.println(payload);
      DynamicJsonDocument doc(512);
      auto error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
      }

      playerSize = 0;
      JsonArray users = doc.as<JsonArray>();
      for (JsonVariant v : users) {
        // serializeJson(v, Serial);
        int playerId = v.as<int>();
        playerPositions[playerSize][0] = playerId;
        playerPositions[playerSize][1] = 0;
        players[playerSize++] = playerId;
      }
      });
    socket->emit("tp_getUsers", String("{ \"game\": \"trivialpursuit\", \"id\": " + String(gameId) + " }").c_str());

    socket->on("tp_getPlaats", [&](const char* payload, size_t len) {
      DynamicJsonDocument doc(512);
      auto error = deserializeJson(doc, payload);
      if (error) {
        return;
      }
      for (int i = 0; i < playerSize; ++i) {
        if(players[i] == "id"){
          setStep(doc[players[i].as<int>(), doc["id"]["plek"].as<int>());
        }
      }
      socket->emit("tp_turnCard", String("{ \"game\": \"trivialpursuit\", \"id\": " + String(gameId) + " }").c_str());
    }
      
      void setStep(int id, int plek) {
        if (id == 0){
          strip->setPixelColor(plek, 50, 0, 0);
          strip->setPixelColor(old_1, 0, 0, 0);
          
          old_1 = plek;
          strip->show();
        }

        else if (id == 1){
          strip->setPixelColor(plek, 0, 50, 0);
          strip->setPixelColor(old_2, 0, 0, 0);

          old_2 = plek;
          strip->show();
        }


        else if (id == 2){
          strip->setPixelColor(plek, 0, 0, 50);
          strip->setPixelColor(old_3, 0, 0, 0);

          old_3 = plek;
          strip->show();
        }

        else if (id == 3){
          strip->setPixelColor(plek, 50, 50, 0);
          strip->setPixelColor(old_4, 0, 0, 0);
          
          old_4 = plek;
          strip->show();
        }
    });
    socket->emit("tp_getPlaats", String("{ \"game\": \"trivialpursuit\", \"id\": " + String(gameId) + " }").c_str());

  void renderPlayers() {
    Serial.println("RENDERPLAYERS");
    strip->fill(0, 0, 0);

    for (int i = 0; i < playerSize; ++i) {
      int c[3] = { 255, 0, 0 };

      switch (i) {
      case 0:
        c[0] = 252;
        c[1] = 14;
        c[2] = 14;
        break;
      case 1:
        c[0] = 236;
        c[1] = 61;
        c[2] = 252;
        break;
      case 2:
        c[0] = 252;
        c[1] = 190;
        c[2] = 42;
        break;
      case 3:
        c[0] = 5;
        c[1] = 100;
        c[2] = 252;
        break;
      }
      
      strip->setPixelColor(steps["stap_" + String(playerPositions[i][1])], c[0], c[1], c[2]);
      Serial.println(steps["stap_" + String(playerPositions[i][1])]);
      Serial.println("stap_" + String(playerPositions[i][1]));
    }
    
    strip->show();
  }

	void loop() {
  strip->show();
	}
}
};
