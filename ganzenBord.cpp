#include "Arduino.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>
#include "ArduinoJson.h" 
#include <map>

class GanzenBord {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int gameId = -1;
	int user_id = -1;

	int players[4];
	int playerPositions[4][2];
	uint8_t playerSize = 0;
	int myTurn = false;
	long lastPrint = 0;
	long printDelay = 1000;
	int myPosition = 0;
	int turn = -1;

	std::map<String, int> steps = {
		{"stap_0", 0},
		{"stap_1", 1},
		{"stap_2", 2},
		{"stap_3", 3},
		{"stap_4", 4},
		{"stap_5", 5},
		{"stap_6", 6},
		{"stap_7", 7},
		{"stap_8", 8},

		{"stap_9", 23},
		{"stap_10", 24},
		{"stap_11", 39},
		{"stap_12", 40},
		{"stap_13", 55},
		{"stap_14", 56},
		{"stap_15", 57},
		{"stap_16", 58},

		{"stap_17", 59},
		{"stap_18", 60},
		{"stap_19", 61},
		{"stap_20", 62},
		{"stap_21", 63},
		{"stap_22", 48},
		{"stap_23", 47},
		{"stap_24", 32},

		{"stap_25", 31},
		{"stap_26", 16},
		{"stap_27", 15},
		{"stap_28", 14},
		{"stap_29", 13},
		{"stap_30", 12},
		{"stap_31", 11},
		{"stap_32", 10},

		{"stap_33", 9},
		{"stap_34", 22},
		{"stap_35", 25},
		{"stap_36", 38},
		{"stap_37", 41},
		{"stap_38", 54},
		{"stap_39", 53},
		{"stap_40", 52},

		{"stap_41", 51},
		{"stap_42", 50},
		{"stap_43", 49},
		{"stap_44", 46},
		{"stap_45", 33},
		{"stap_46", 30},
		{"stap_47", 17},
		{"stap_48", 18},

		{"stap_49", 19},
		{"stap_50", 20},
		{"stap_51", 21},
		{"stap_52", 26},
		{"stap_53", 37},
		{"stap_54", 42},
		{"stap_55", 43},
		{"stap_56", 44},

		{"stap_57", 45},
		{"stap_58", 34},
		{"stap_59", 29},
		{"stap_60", 28},
		{"stap_61", 27},
		{"stap_62", 36},
		{"stap_63", 35},
	};
public:
	void setSocket(SocketIoClient* s) { this->socket = s; }
	void setStrip(Adafruit_NeoPixel* strip) { this->strip = strip; }
	void setGameId(int gameId) { if (this->gameId != gameId) this->gameId = gameId; }

	void setup() {
		Serial.println("Ganzenbord!");
		renderPlayers();
		strip->setBrightness(169);

		socket->on("user_id", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(len * 2);
			auto error = deserializeJson(doc, payload);
			if (error) {
				Serial.print(F("deserializeJson() failed with code "));
				Serial.println(error.c_str());
				return;
			}
			user_id = doc["user_id"].as<int>();
			Serial.println("user_id");
			Serial.println(user_id);
			});
		socket->emit("user_id", String("{ \"game\": \"ganzenbord\", \"id\": " + String(gameId) + " }").c_str());

		socket->on("getUsers", [&](const char* payload, size_t len) {
			Serial.println("GETUSERS");
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

			for (int i = 0; i < playerSize; ++i) {
				Serial.println(players[i]);
			}
			// players = data;
			// socket.emit('ganzenbord_state', { game: game, id: id });
			});
		socket->emit("getUsers", String("{ \"game\": \"ganzenbord\", \"id\": " + String(gameId) + " }").c_str());

		socket->on("turn", [&](const char* payload, size_t len) {
			Serial.println("TURN EVENT");
			Serial.println(payload);
			DynamicJsonDocument doc(512);
			auto error = deserializeJson(doc, payload);
			if (error) {
				Serial.print(F("deserializeJson() failed with code "));
				Serial.println(error.c_str());
				return;
			}

			// animationPlayerTurn = true;

			int _turn = doc["turn"].as<int>();
			turn = _turn;

			if (user_id == _turn) myTurn = true;
			else myTurn = false;
			// if (turn != user_id) myTurn = false;
			// else myTurn = true;
			});

		socket->on("ganzenbord_state", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(512);
			auto error = deserializeJson(doc, payload);
			if (error) {
				Serial.print(F("deserializeJson() failed with code "));
				Serial.println(error.c_str());
				return;
			}

			// {"playerPositions":{"3":35,"1":33},"skipBeurtPlayers":[]}

			// JsonArray users = doc["playerPositions"].as<JsonArray>();
			Serial.println("GANZENBORD STATE!");
			Serial.println(payload);
			for (int i = 0; i < playerSize; ++i) {
				uint8_t pos = doc["playerPositions"][String(players[i])].as<uint8_t>();
				Serial.println(playerPositions[i][0]);
				Serial.println(players[i]);
				Serial.println(pos);
				Serial.println(playerPositions[i][1]);
				if (playerPositions[i][0] == players[i]) {
					playerPositions[i][1] = pos;
				}
				// strip->setPixelColor(steps["step_" + String(playerPositions[i])]);
			}

			// for (JsonVariant v : users) {
			// 	// serializeJson(v, Serial);
			// 	int playerId = v.as<int>();
			// 	playerPositions[playerSize][0] = playerId;
			// 	playerPositions[playerSize][1] = 0;
			// 	players[playerSize++] = playerId;
			// }

			});
		socket->emit("ganzenbord_state", String("{ \"game\": \"ganzenbord\", \"id\": " + String(gameId) + " }").c_str());

		socket->on("dobbel", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(512);
			auto error = deserializeJson(doc, payload);
			if (error) {
				Serial.print(F("deserializeJson() failed with code "));
				Serial.println(error.c_str());
				return;
			}
			Serial.println("GANZENBORD DOBBEL!");
			Serial.println(payload);

			// {"getal":7,"position":63,"playerId":"1"}

			int playerId = doc["playerId"].as<int>();
			uint8_t position = doc["position"].as<int>();

			for (int i = 0; i < playerSize; ++i) {
				if (playerPositions[i][0] == playerId) {
					playerPositions[i][1] = position;
					break;
				}
				Serial.println(playerPositions[i][1]);
				// strip->setPixelColor(steps["step_" + String(playerPositions[i])]);
			}

			// TODO: gotwo
			renderPlayers();
			});
	}

	void gotwo(int player, uint8_t position) {
		for (int i = 0; i < playerSize; ++i) {
			if (playerPositions[i][0] == player) {
				playerPositions[i][1] = position;
			}
			// strip->setPixelColor(steps["step_" + String(playerPositions[i])]);
		}
		// strip->setPixelColor(steps["step_" + String(position)], 255, 0, 0); // TODO: player color
		// strip->show();
		renderPlayers();
	}

	void renderPlayers() {
		Serial.println("RENDERPLAYERS");
		strip->fill(0, 0, 0);
		// uint8_t currentPlayerPosition = 0;

		// int cpc[3] = {255, 0, 0};

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
			// if (playerPositions[i][0] == turn) {
			// 	currentPlayerPosition = playerPositions[i][1];
			// 	memcpy(&cpc, c, 3);
			// }

			strip->setPixelColor(steps["stap_" + String(playerPositions[i][1])], c[0], c[1], c[2]);
			Serial.println(steps["stap_" + String(playerPositions[i][1])]);
			Serial.println("stap_" + String(playerPositions[i][1]));
		}

		// strip->setPixelColor(steps["stap_" + String(currentPlayerPosition)], cpc[0], cpc[1], cpc[2]);
		strip->show();
	}

	int touchReadAvg(int pin) {
		long value = 0;

		for (int i = 0; i < 10; i++) {
			value += touchRead(pin);
		}

		return value / 10;
	}

	void processInputs() {
		// TODO: Dobbel
		if (touchReadAvg(T0) < 25) {
			Serial.println("Touch 0");
		}
		else if (touchReadAvg(T8) < 25) {
			myTurn = false;
			Serial.println("DOBBEL");
			socket->emit("dobbel", String("{ \"game\": \"ganzenbord\", \"id\": " + String(gameId) + " }").c_str());
			Serial.println("Touch 1");
		}
		else if (touchReadAvg(T3) < 25) {
			Serial.println("Touch 2");
		}
		else if (touchReadAvg(T4) < 25) {
			Serial.println("Touch 3");
		}
		else if (touchReadAvg(T5) < 25) {
			Serial.println("Touch 4");
		}
		else if (touchReadAvg(T6) < 25) {
			Serial.println("Touch 5");
		}
		else if (touchReadAvg(T7) < 25) {
			Serial.println("Touch 6");
		}
		else if (touchReadAvg(T9) < 25) {
			Serial.println("Touch 7");
		}
	}

	void loop() {
		long now = millis();
		if (myTurn) processInputs();

		if (now >= lastPrint + printDelay) {
			lastPrint = now;
			renderPlayers();
			/*
				// UNCOMMENT OM TE CHECKEN OF DE LEDSTRIP WERKT

				strip->setPixelColor(63, 0, 255, 0);
				strip->show();
			*/

			// Serial.println("PRINT");
			// Serial.println(myTurn);
			// Serial.printf("T0=%d\n", touchReadAvg(T0));
			// Serial.printf("T2=%d\n", touchReadAvg(T2));
			// Serial.printf("T3=%d\n", touchReadAvg(T3));
			// Serial.printf("T4=%d\n", touchReadAvg(T4));
			// Serial.printf("T5=%d\n", touchReadAvg(T5));
			// Serial.printf("T6=%d\n", touchReadAvg(T6));
			// Serial.printf("T7=%d\n", touchReadAvg(T7));
			// Serial.printf("T8=%d\n", touchReadAvg(T8));
			// Serial.printf("T9=%d\n", touchReadAvg(T9));
		}
	}
};