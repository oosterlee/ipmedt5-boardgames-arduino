#include "Arduino.h"
#include "ArduinoJson.h"
#include "SocketIoClient.h"
#include <Adafruit_NeoPixel.h>

class VierOpEenRij {
private:
	SocketIoClient* socket;
	Adafruit_NeoPixel* strip;
	int user_id = -1;
	bool inGame = false;
	int gameId = -1;
	bool winner = false;
	int winnerId = -1;
	bool myTurn = false;

	long lastPrint = 0;
	long printDelay = 1000;

	int board[8][8] = {
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1, -1, -1, -1},
	};
public:
	void setSocket(SocketIoClient* s) {this->socket = s;}
	void setStrip(Adafruit_NeoPixel* strip) {this->strip = strip;}
	void setGameId(int gameId) {if (this->gameId != gameId) this->gameId = gameId;}

	void setup() {
		Serial.println("[SETUP] Vier op een rij!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		resetBoard();

		socket->on("user_id", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(len*2);
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
		socket->emit("user_id", "");

		socket->on("fiar_place", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(len*2);
			auto error = deserializeJson(doc, payload);
			if (error) {
			    Serial.print(F("deserializeJson() failed with code "));
			    Serial.println(error.c_str());
			    return;
			}

			place(doc["column"].as<int>(), doc["user"].as<int>());
		});

		socket->on("fiar_state", [&](const char* payload, size_t len) {
			DynamicJsonDocument doc(4096);
			auto error = deserializeJson(doc, payload);
			if (error) {
			    Serial.print(F("deserializeJson() failed with code "));
			    Serial.println(error.c_str());
			    return;
			}
			resetBoard();

			JsonArray actions = doc.as<JsonArray>();

			for (JsonVariant v : actions) {
				String action = v["action"].as<String>();
				if (action == "fiar_place") {
					Serial.println("IT IS PLACE");

					place(v["column"].as<int>(), v["player"].as<int>());
				}
			}
		});
		socket->emit("fiar_state", String("{ \"game\": \"vieropeenrij\", \"id\": " + String(gameId) + " }").c_str());

		socket->on("turn", [&](const char* payload, size_t len) {
			Serial.println("TURN EVENT");
			Serial.println(payload);
			DynamicJsonDocument doc(len*2);
			auto error = deserializeJson(doc, payload);
			if (error) {
			    Serial.print(F("deserializeJson() failed with code "));
			    Serial.println(error.c_str());
			    return;
			}

			int turn = doc["turn"].as<int>();
			if (turn != user_id) myTurn = false;
			else myTurn = true;
		});
		// socket->on("message", [](const char* payload, size_t len) {
		// 	Serial.println("VoeR msg");
		// 	Serial.println(payload);
		// });

		// strip->setPixelColor(0, 0, 0, 255);

		// for (int i = 0; i < sizeof(board) / sizeof(int); i++) {
		// 	colrowSetPixel(0, i, 255, 0, 0);
		// 	colrowSetPixel(7, i, 255, 0, 0);
		// }
		// strip->show();
		renderBoard();
	}

	void renderBoard() {
		Serial.println("RENDERBOARD");
		strip->fill(0, 0, 0);
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				if (board[r][c] == -1) continue;
				Serial.printf("c=%d, r=%d, board=%d, user=%d", c, r, board[r][c], user_id);
				if (board[r][c] == user_id) colrowSetPixel(c, r, 0, 0, 255);
				else colrowSetPixel(c, r, 255, 0, 0);
			}
		}
		strip->show();
	}

	void showUser() {
		// TODO: Show what users turn it is by replacing the center ?4? spaces with that color and show this for a few seconds. Maybe blink it?
	}

	void colrowSetPixel(uint8_t col, uint8_t row, uint8_t r, uint8_t g, uint8_t b) {
		strip->setPixelColor(colrowToPixel(col, row), r, g, b);
	}

	uint8_t colrowToPixel(uint8_t col, uint8_t row) {
		return row % 2 == 0 ? (col + (8 * row)) : ((8 * row) + 8) - 1 - col;
		// return row % 2 == 0 ? ((8 * row) + 8) - 1 - col : (col + (8 * row)); // FLIPPED
	}

	void place(uint8_t col, int player) {
		Serial.println("PLACE");
		Serial.println(col);
		Serial.println(player);
		if (winner && player == -1) return;
		if (player == -1) {
			myTurn = false;
			socket->emit("fiar_place", String("{ \"column\": " + String(col) + ", \"game\": \"vieropeenrij\", \"id\" \"" + gameId + "\" }").c_str());
		}

		uint8_t lic = getLastInColumn(col);
		if (lic == -1) return;

		board[lic][col] = player == -1 ? user_id : player;
		renderBoard();
	}

	uint8_t getLastInColumn(uint8_t col) {
		for (int r = 8; r >= 0; r--) {
			if (board[r][col] == -1) return r;
		}

		return -1;
	}

	void resetBoard() {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				board[i][j] = -1;
			}
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
		if (touchReadAvg(T0) < 25) {
			place(0, -1);
		} else if (touchReadAvg(T2) < 25) {
			place(1, -1);
		} else if (touchReadAvg(T2) < 25) {
			place(2, -1);
		} else if (touchReadAvg(T3) < 25) {
			place(3, -1);
		} else if (touchReadAvg(T4) < 25) {
			place(4, -1);
		} else if (touchReadAvg(T5) < 25) {
			place(5, -1);
		} else if (touchReadAvg(T6) < 25) {
			place(6, -1);
		} else if (touchReadAvg(T7) < 25) {
			place(7, -1);
		} else if (touchReadAvg(T9) < 25) {
			place(8, -1);
		}
	}

	void loop() {
		long now = millis();

		if (myTurn) processInputs();

		// if (now >= lastPrint+printDelay) {
		// 	lastPrint = now;
		// 	Serial.println("PRINT");
		// 	Serial.printf("T0=%d\n", touchReadAvg(T0));
		// 	Serial.printf("T2=%d\n", touchReadAvg(T2));
		// 	Serial.printf("T3=%d\n", touchReadAvg(T3));
		// 	Serial.printf("T4=%d\n", touchReadAvg(T4));
		// 	Serial.printf("T5=%d\n", touchReadAvg(T5));
		// 	Serial.printf("T6=%d\n", touchReadAvg(T6));
		// 	Serial.printf("T7=%d\n", touchReadAvg(T7));
		// 	Serial.printf("T8=%d\n", touchReadAvg(T8));
		// 	Serial.printf("T9=%d\n", touchReadAvg(T9));
		// }
	}
};

// 4
// 2
// 15
// 13
// 12
// 14
// 27
// 33