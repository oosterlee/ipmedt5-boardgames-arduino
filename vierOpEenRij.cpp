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

	typedef enum animation_t {
		ANIMATION_NONE,
		ANIMATION_PIECE_FALL,
		ANIMATION_PLAYER_TURN,
		ANIMATION_PLAYER_WIN,
	} animation_t;

	bool animationPieceFall = false;
	bool animationPlayerTurn = false;
	bool animationPlayerWin = false;

	bool animationPieceFallDone = true;
	long lastAnimationPieceFall = 0;
	long animationPieceFallDelay = 200;
	int animationPieceFallStep = 0;
	int animationPieceRow = -1;
	int animationPiecePlayer = -1;
	uint8_t animationPieceCol = 0;

	int winningPiecesVertical[2][2] = { {-1, -1}, {-1, -1} };
	int winningPiecesHorizontal[2][2] = { {-1, -1}, {-1, -1} };

	long lastAnimationPlayerTurn = 0;
	long animationPlayerTurnDelay = 1000;
	bool animationPlayerTurnState = false;

	long lastAnimationPlayerWin = 0;
	long animationPlayerWinDelay = 250;
	bool animationPlayerWinState = false;

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

		strip->setBrightness(96);

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
			DynamicJsonDocument doc(len*4);
			auto error = deserializeJson(doc, payload);
			if (error) {
			    Serial.print(F("deserializeJson() failed with code "));
			    Serial.println(error.c_str());
			    return;
			}

			animationPlayerTurn = true;

			int turn = doc["turn"].as<int>();
			if (turn != user_id) myTurn = false;
			else myTurn = true;
		});

		socket->on("fiar_winner", [&](const char* payload, size_t len) {
			Serial.println("FIAR WINNER!");
			DynamicJsonDocument doc(1024);
			auto error = deserializeJson(doc, payload);
			if (error) {
			    Serial.print(F("deserializeJson() failed with code "));
			    Serial.println(error.c_str());
			    return;
			}

			Serial.println(payload);

			JsonObject vertical = doc["winningPieces"]["vertical"];
			JsonObject horizontal = doc["winningPieces"]["horizontal"];

			if (!vertical.isNull()) {
				Serial.println("VERTICAL");
				JsonArray verticalArrB = vertical["begin"].as<JsonArray>();
				winningPiecesVertical[0][0] = verticalArrB[0].as<int>();
				winningPiecesVertical[0][1] = verticalArrB[1].as<int>();

				JsonArray verticalArrE = vertical["end"].as<JsonArray>();
				winningPiecesVertical[1][0] = verticalArrE[0].as<int>();
				winningPiecesVertical[1][1] = verticalArrE[1].as<int>();

				Serial.println(verticalArrB[0].as<int>());
				Serial.println(verticalArrB[1].as<int>());
				Serial.println(verticalArrE[0].as<int>());
				Serial.println(verticalArrE[1].as<int>());

				winner = true;
				animationPlayerWin = true;
				animationPlayerTurn = false;
			}
			if (!horizontal.isNull()) {
				Serial.println("HORIZONTAL");
				JsonArray horizontalArrB = horizontal["begin"].as<JsonArray>();
				winningPiecesHorizontal[0][0] = horizontalArrB[0].as<int>();
				winningPiecesHorizontal[0][1] = horizontalArrB[1].as<int>();

				JsonArray horizontalArrE = horizontal["end"].as<JsonArray>();
				winningPiecesHorizontal[1][0] = horizontalArrE[0].as<int>();
				winningPiecesHorizontal[1][1] = horizontalArrE[1].as<int>();

				Serial.println(horizontalArrB[0].as<int>());
				Serial.println(horizontalArrB[1].as<int>());
				Serial.println(horizontalArrE[0].as<int>());
				Serial.println(horizontalArrE[1].as<int>());
				winner = true;
				animationPlayerWin = true;
				animationPlayerTurn = false;
			}
			// winningPiecesVertical
			// winningPiecesHorizontal
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
				// Serial.printf("c=%d, r=%d, board=%d, user=%d", c, r, board[r][c], user_id);
				if (board[r][c] == user_id) colrowSetPixel(c, r, 0, 0, 255);
				else colrowSetPixel(c, r, 255, 0, 0);
				// Serial.printf("c=%d, r=%d, user=%d\n", c, r, board[r][c]);
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
			socket->emit("fiar_place", String("{ \"column\": " + String(col) + ", \"game\": \"vieropeenrij\", \"id\": \"" + gameId + "\" }").c_str());
		}

		if (animationPieceFallDone == false) {
			board[animationPieceRow][animationPieceCol] = animationPiecePlayer;
		}
		uint8_t lic = getLastInColumn(col);
		if (lic == -1) return;

		// board[lic][col] = player == -1 ? user_id : player;
		animationPiecePlayer = player == -1 ? user_id : player;
		animationPieceRow = lic;
		animationPieceFall = true;
		animationPieceFallStep = 0;
		animationPieceCol = col;
		animationPieceFallDone = false;
		renderBoard();
	}

	uint8_t getLastInColumn(uint8_t col) {
		for (int r = 8; r >= 0; r--) {
			if (board[r][col] == -1) return r;
		}

		return -1;
	}

	void resetBoard() {
		animationPieceFall = false;
		animationPlayerTurn = false;
		animationPlayerWin = false;
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
		} else if (touchReadAvg(T3) < 25) {
			place(2, -1);
		} else if (touchReadAvg(T4) < 25) {
			place(3, -1);
		} else if (touchReadAvg(T5) < 25) {
			place(4, -1);
		} else if (touchReadAvg(T6) < 25) {
			place(5, -1);
		} else if (touchReadAvg(T7) < 25) {
			place(6, -1);
		} else if (touchReadAvg(T9) < 25) {
			place(7, -1);
		}
	}

	// void animate(animation_t animation) {
	// 	if (animation == ANIMATION_NONE) return;
	// 	if (oldAnimation != animation) {
	// 		animationStep = 0;
	// 		lastAnimation = 0;
	// 	}
	// 	else animationStep++;

	// 	long now = millis();
	// 	if (now >= lastAnimation+animationDelays[animation]) {
	// 		Serial.println("Animation!");
	// 	}
	// }

	void animationPieceFallFunc(long now) {
		if (animationPieceRow == -1) return;
		lastAnimationPieceFall = now;
		Serial.println("animation fall");
		if (animationPieceFallStep >= animationPieceRow) {
			animationPieceFallDone = true;
			animationPieceFall = false;
			// Serial.println("Doneeee");
			Serial.printf("ac=%d, ar=%d\n", animationPieceCol, animationPieceRow);
			// Serial.println(animationPiecePlayer);
			board[animationPieceRow][animationPieceCol] = animationPiecePlayer;
			renderBoard();
		} else {
			if (animationPieceFallStep > 0) {
				colrowSetPixel(animationPieceCol, animationPieceFallStep-1, 0, 0, 0);
			}
			if (user_id == animationPiecePlayer) colrowSetPixel(animationPieceCol, animationPieceFallStep, 0, 0, 255);
			else colrowSetPixel(animationPieceCol, animationPieceFallStep, 255, 0, 0);
			strip->show();
			animationPieceFallStep++;
			// Serial.println(animationPieceFallStep++);
			Serial.printf("c=%d, r=%d\n", animationPieceCol, animationPieceFallStep);
		}
	}

	void animationPlayerTurnFunc(long now) {
		Serial.println("animationPlayerTurnFunc");
		lastAnimationPlayerTurn = now;
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				if (board[r][c] == -1) continue;
				// Serial.printf("c=%d, r=%d, board=%d, user=%d", c, r, board[r][c], user_id);
				if (myTurn && board[r][c] == user_id) colrowSetPixel(c, r, 0, 0, animationPlayerTurnState ? 255 : 96);
				else if(!myTurn && board[r][c] != user_id) colrowSetPixel(c, r, animationPlayerTurnState ? 255 : 96, 0, 0);
				// Serial.printf("c=%d, r=%d, user=%d\n", c, r, board[r][c]);
			}
		}
		animationPlayerTurnState = !animationPlayerTurnState;
		strip->show();
	}

	void animationPlayerWinFunc(long now) {
		lastAnimationPlayerWin = now;

			Serial.println("ANIMATIONPLAYERWINFUNC");
		if (winningPiecesVertical[0][0] != -1) {
			for (int i = winningPiecesVertical[1][1]; i < winningPiecesVertical[0][1]+1; i++) {
				if (board[i][winningPiecesVertical[0][0]] == user_id) colrowSetPixel(winningPiecesVertical[0][0], i, 0, 0, animationPlayerWinState ? 255 : 96);
				else if (board[i][winningPiecesVertical[0][0]] != -1) colrowSetPixel(winningPiecesVertical[0][0], i, animationPlayerWinState ? 255 : 96, 0, 0);
			}
		}

		Serial.printf("bc=%d, ec=%d, br=%d, er=%d\n",
			winningPiecesHorizontal[0][0],
			winningPiecesHorizontal[1][0],
			winningPiecesHorizontal[0][1],
			winningPiecesHorizontal[1][1]);

		if (winningPiecesHorizontal[0][0] != -1) {
			for (int i = winningPiecesHorizontal[0][0]; i < winningPiecesHorizontal[1][0]+1; i++) {
				if (board[winningPiecesHorizontal[0][1]][i] == user_id) colrowSetPixel(i, winningPiecesHorizontal[0][1], 0, 0, animationPlayerWinState ? 255 : 96);
				else if (board[winningPiecesHorizontal[0][1]][i] != -1) colrowSetPixel(i, winningPiecesHorizontal[0][1], animationPlayerWinState ? 255 : 96, 0, 0);
			}
		}


		// for (let i = winningPieces.horizontal.begin[0]; i < winningPieces.horizontal.end[0]+1; i++) {
		// 		const piece = getPiecePlace(i, winningPieces.horizontal.begin[1]);
		// 		if (!piece && timeout == true) {
		// 			timeout = false;
		// 			return setTimeout(winnerText, 25);
		// 		}
		// 		console.log(piece, i, winningPieces.horizontal.begin[1]);
		// 		piece.classList.add("js--piece-win");
		// 		console.log("WIN", i);
		// 	}
		strip->show();

		animationPlayerWinState = !animationPlayerWinState;


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

		if (now >= lastAnimationPieceFall+animationPieceFallDelay && (animationPieceFall == true || animationPieceFallDone == false)) {
			animationPieceFallFunc(now);
		}

		if (now >= lastAnimationPlayerTurn+animationPlayerTurnDelay && animationPlayerTurn == true) {
			animationPlayerTurnFunc(now);
			// Serial.println("animation playerturn");
		}

		if (now >= lastAnimationPlayerWin+animationPlayerWinDelay && animationPlayerWin == true) {
			animationPlayerWinFunc(now);
			// Serial.println("animation playerwin");
		}

		// long lastAnimationPieceFall = 0;
		// long animationPieceFallDelay = 200;
		// long animationPieceFallDone = false;

		// long lastAnimationPlayerTurn = 0;
		// long animationPlayerTurnDelay = 200;

		// long lastAnimationPlayerWin = 0;
		// long animationPlayerWinDelay = 200;
	}
};