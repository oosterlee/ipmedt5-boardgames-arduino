#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "ArduinoJson.h"
#include <Preferences.h>

#include "WiFiManager.h"

// #include <WebSocketsClient.h>
#include "config.h"
#include "SocketIoClient.h"

#include "vierOpEenRij.cpp"
#include "thirtySeconds.cpp"
#include "trivialPursuit.cpp"
#include "vlotteGeesten.cpp"
#include "ganzenBord.cpp"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Preferences preferences;

WiFiManager wifiManager;
WiFiManagerParameter laravelusername("laravelusername", "Laravel Username", "", 40);
WiFiManagerParameter laravelpassword("laravelpassword", "Laravel Password", "", 40);

SocketIoClient sioc;

bool shouldSaveConfig = false;

long lastSetReset = 0;
long delayReset = 1000;

typedef enum current_game_t {
	GAME_NONE,
	GAME_VIEROPEENRIJ,
	GAME_THIRTYSECONDS,
	GAME_TRIVIALPURSUIT,
	GAME_VLOTTEGEESTEN,
	GAME_GANZENBORD
} current_game_t;

#ifdef TEST_GAME
current_game_t currentGame = TEST_GAME;
#else
current_game_t currentGame = GAME_NONE;
#endif
bool currentGameSetup = false;

VierOpEenRij gameVierOpEenRij;
ThirtySeconds gameThirtySeconds;
TrivialPursuit gameTrivialPursuit;
VlotteGeesten gameVlotteGeesten;
GanzenBord gameGanzenbord;

int gameId = -1;


void saveConfigCallback () {
	Serial.println("Should save config");
	shouldSaveConfig = true;
}

void wifiConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	Serial.println("[wifiConnected]");
	Serial.println((char *)info.connected.ssid);
}

void setup() {
	Serial.begin(115200);
	// Serial.setDebugOutput(true);
	Serial.println("");
	Serial.println("Setup");
	preferences.begin("credentials", false);
	int count = preferences.getInt("resetCount", 0);
	preferences.putInt("resetCount", count + 1);

	Serial.println("RESET COUNT");
	Serial.println(count);

	if (count >= 4) {
		Serial.println("Resetting!");
		wifiManager.resetSettings();
		preferences.remove("resetCount");
		ESP.restart();
		return;
	}

	strip.begin();

	strip.setBrightness(8);

  	strip.show();

	Serial.println("Nothing found!");

	WiFi.onEvent(wifiConnected, SYSTEM_EVENT_STA_CONNECTED);

	strip.setPixelColor(0, 255, 0, 0);
	strip.show();


 	wifiManager.addParameter(&laravelusername);
 	wifiManager.addParameter(&laravelpassword);

 	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.setCaptivePortalEnable(true);

	if (!wifiManager.autoConnect("boardgame-setup", "boardgame123")) {
		strip.setPixelColor(0, 255, 69, 0);
		strip.show();
		delay(9999999);
		ESP.restart();
	}

  	String laravelUsername = preferences.getString("lu", "");
  	String laravelPassword = preferences.getString("lp", "");

  	if (!shouldSaveConfig && (laravelUsername == "" || laravelPassword == "")) {
  		// Blink some led??
  		Serial.println("Laravel Username and Laravel Password is not defined!");
  		wifiManager.resetSettings();
  		return ESP.restart();
  	} else if (shouldSaveConfig) {
		laravelUsername = laravelusername.getValue();
  		laravelPassword = laravelpassword.getValue();
		preferences.putString("lu", laravelUsername);
		preferences.putString("lp", laravelPassword);
  	}

  	// Connect to laravel backend with laravelusername && laravelpassword
  	Serial.print("Connected to WiFi and got laravel credentials:: username=");
  	Serial.print(laravelUsername);
  	Serial.print(",password=");
  	Serial.println(laravelPassword);

	strip.setPixelColor(0, 0, 255, 0);
	strip.show();
	delay(1000);

	WiFi.mode(WIFI_STA);

	sioc.on("connect", [](const char* payload, size_t len) {
		Serial.println("Socketio connected");
		sioc.emit("hardware", "{ \"hardware\": true }");
	});
	sioc.on("login", [](const char* payload, size_t len) {
		Serial.println("[ubg] LOGIN");
		Serial.println(payload);
	});

	sioc.on("game", [](const char* payload, size_t len) {
		DynamicJsonDocument doc(len*2);
		auto error = deserializeJson(doc, payload);
		if (error) {
		    Serial.print(F("deserializeJson() failed with code "));
		    Serial.println(error.c_str());
		    return;
		}
		Serial.println("[ubg] GAME");
		String game = doc["game"].as<String>();
		Serial.println(String(game));

		if (game == "false") {
			Serial.println("GAME IS FALSE");
		} else {
			Serial.println("WE ARE IN A GAME");
			int oldGameId = gameId;
			if(game == "vieropeenrij") currentGame = GAME_VIEROPEENRIJ;
			else if(game == "thirtyseconds") currentGame = GAME_THIRTYSECONDS;
			else if(game == "trivialpursuit") currentGame = GAME_TRIVIALPURSUIT;
			else if(game == "vlottegeest") currentGame = GAME_VLOTTEGEESTEN;
			else if(game == "ganzenbord") currentGame = GAME_GANZENBORD;
			else {
				currentGameSetup = false;
				currentGame = GAME_NONE;
				// gameId = -1;
			}

			gameId = doc["id"].as<int>();
			if (oldGameId != gameId) {
				sioc.emit("leave_session", String("{ \"game\": " + game + ", \"id\": " + String(oldGameId) + "}").c_str());
				currentGameSetup = false;
				sioc.emit("join_session", String("{ \"game\": \"" + game + "\", \"id\": " + String(gameId) + "}").c_str());
			}

			Serial.println(currentGame);
			Serial.println(gameId);

		}
		// Serial.println(doc["loggedIn"]);
	});

	sioc.on("hardware", [](const char* payload, size_t len) {
		DynamicJsonDocument doc(len*2);
		auto error = deserializeJson(doc, payload);
		if (error) {
		    Serial.print(F("deserializeJson() failed with code "));
		    Serial.println(error.c_str());
		    return;
		}

		if (doc["hardware"].as<bool>() == false) {
			sioc.emit("hardware", "{ \"hardware\": true }");
		}
		Serial.println("[ubg] HARDWARE");
		Serial.println(payload);
	});

#if defined(TEST_NO_SIO)
#if defined(TEST_WS_URL) && defined(TEST_WS_PORT)
	Serial.println(TEST_WS_URL);
	Serial.println(TEST_WS_PORT);
	sioc.begin(TEST_WS_URL, TEST_WS_PORT, String("/socket.io/?email=" + laravelUsername + "&password=" + laravelPassword).c_str());
#elif defined(TEST_WS_URL)
	Serial.println(TEST_WS_URL);
	Serial.println(WEBSOCKET_PORT);
	sioc.begin(TEST_WS_URL, WEBSOCKET_PORT, String("/socket.io/?email=" + laravelUsername + "&password=" + laravelPassword).c_str());
#elif defined(TEST_WS_PORT)
	Serial.println(WEBSOCKET_URL);
	Serial.println(TEST_WS_PORT);
	sioc.begin(WEBSOCKET_URL, TEST_WS_PORT, String("/socket.io/?email=" + laravelUsername + "&password=" + laravelPassword).c_str());
#else
	Serial.println(WEBSOCKET_URL);
	Serial.println(WEBSOCKET_PORT);
	sioc.begin(WEBSOCKET_URL, WEBSOCKET_PORT, String("/socket.io/?email=" + laravelUsername + "&password=" + laravelPassword).c_str());
#endif
#endif
}

void loop() {
	sioc.loop();
	long now = millis();

	if (now >= lastSetReset+delayReset) {
		Serial.println("REMOVE RESET COUNT");
		lastSetReset = 999999999;
		delayReset = 999999999;
		preferences.remove("resetCount");
	}

	switch(currentGame) {
		case GAME_VIEROPEENRIJ:
			if (!currentGameSetup) {
				gameVierOpEenRij.setSocket(&sioc);
				gameVierOpEenRij.setStrip(&strip);
				gameVierOpEenRij.setGameId(gameId);
				currentGameSetup = true;
				gameVierOpEenRij.setup();
			}
			gameVierOpEenRij.loop();
			break;
		case GAME_THIRTYSECONDS:
			if (!currentGameSetup) {
				gameThirtySeconds.setSocket(&sioc);
				gameThirtySeconds.setStrip(&strip);
				gameThirtySeconds.setGameId(gameId);
				currentGameSetup = true;
				gameThirtySeconds.setup();
			}
			gameThirtySeconds.loop();
			break;
		case GAME_TRIVIALPURSUIT:
			if (!currentGameSetup) {
				gameTrivialPursuit.setSocket(&sioc);
				gameTrivialPursuit.setStrip(&strip);
				gameTrivialPursuit.setGameId(gameId);
				currentGameSetup = true;
				gameTrivialPursuit.setup();
			}
			gameTrivialPursuit.loop();
			break;
		case GAME_VLOTTEGEESTEN:
			if (!currentGameSetup) {
				gameVlotteGeesten.setSocket(&sioc);
				gameVlotteGeesten.setStrip(&strip);
				gameVlotteGeesten.setGameId(gameId);
				currentGameSetup = true;
				gameVlotteGeesten.setup();
			}
			gameVlotteGeesten.loop();
			break;
		case GAME_GANZENBORD:
			if (!currentGameSetup) {
				gameGanzenbord.setSocket(&sioc);
				gameGanzenbord.setStrip(&strip);
				gameGanzenbord.setGameId(gameId);
				currentGameSetup = true;
				gameGanzenbord.setup();
			}
			gameGanzenbord.loop();
			break;
	}
}