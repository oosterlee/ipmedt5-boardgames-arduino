#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>
#include "ArduinoJson.h" 
#include <map>

class ThirtySeconds {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
  int team_id = -1;
  int pos_old_red = 0;
  int pos_old_blue = 0;

  std::map<String, int> steps = {
    {"stap_0", 20},
    {"stap_1", 21},
    {"stap_2", 22},
    {"stap_3", 23},
    {"stap_4", 8},
    {"stap_5", 7},
    {"stap_6", 6},
    {"stap_7", 5},
    {"stap_8", 4},
    {"stap_9", 3},
    {"stap_10", 2},
    {"stap_11", 1},
    {"stap_12", 0},
    {"stap_13", 15},
    {"stap_14", 16},
    {"stap_15", 17},
    {"stap_16", 18},
    {"stap_17", 29},
    {"stap_18", 34},
    {"stap_19", 45},
    {"stap_20", 46},
    {"stap_21", 47},
    {"stap_22", 48},
    {"stap_23", 63},
    {"stap_24", 62},
    {"stap_25", 61},
    {"stap_26", 60},
    {"stap_27", 59},
    {"stap_28", 58},
    {"stap_29", 57},
    {"stap_30", 56},
    {"stap_31", 55},
    {"stap_32", 40},
    {"stap_33", 41},
    {"stap_34", 42},
    {"stap_35", 43},
  };

public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

  void setup() {
    Serial.println("30 seconds!");
    strip->fill(0, 0, 0);
    strip->setPixelColor(steps["stap_0"], 255, 0, 0);
    strip->show();
    socket->on("teamAnswer", [&](const char* payload, size_t len) {
      DynamicJsonDocument doc(512);
      auto error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return;
      }
      setStep(doc["teamId"].as<int>(), doc["teamInfo"]["position"].as<int>());
    });
  }

  void setStep(int team_id, int team_position) {
    if (team_id == 0){
      if (pos_old_red == pos_old_blue){
        strip->setPixelColor(steps["stap_" + String(pos_old_red)], 0, 0, 255);
      } else{
        strip->setPixelColor(steps["stap_" + String(pos_old_red)], 0, 0, 0);
      }
      strip->setPixelColor(steps["stap_" + String(team_position)], 255, 0, 0);
      pos_old_red = team_position;
      strip->show();
    } else {
      if (pos_old_blue == pos_old_red){
        strip->setPixelColor(steps["stap_" + String(pos_old_blue)], 255, 0, 0);
      } else{
        strip->setPixelColor(steps["stap_" + String(pos_old_blue)], 0, 0, 0);
      }
      strip->setPixelColor(steps["stap_" + String(team_position)], 0, 0, 255);
      pos_old_blue = team_position;
      strip->show();
    }
  }

  int touchReadAvg(int pin) {
    long value = 0;

    for (int i = 0; i < 10; i++) {
      value += touchRead(pin);
    }

    return value / 10;
  }

  void processInputs() {
    if (touchReadAvg(T8) < 25) {
      socket->emit("ts_turncard", String("{ \"game\": \"thirtyseconds\", \"id\": " + String(gameId) + " }").c_str());
      Serial.println("Touch pin");
    }
  }

  void loop() {
    processInputs();
  }
};
