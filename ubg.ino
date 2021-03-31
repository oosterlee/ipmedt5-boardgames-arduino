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
	Serial.println("");
	Serial.println("Setup");
	preferences.begin("credentials", false);
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
  		// wifiManager.resetSettings();
  		// return;
  	} else {
		preferences.putString("lu", laravelusername.getValue());
		preferences.putString("lp", laravelpassword.getValue());
		laravelUsername = laravelusername.getValue();
  		laravelPassword = laravelpassword.getValue();
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
	});

	Serial.println(TEST_WS_URL);
	Serial.println(TEST_WS_PORT);

#if defined(TEST_WS_URL) && defined(TEST_WS_PORT)
	sioc.begin(TEST_WS_URL, TEST_WS_PORT);
#elif defined(TEST_WS_URL)
	sioc.begin(TEST_WS_URL, WEBSOCKET_PORT);
#elif defined(TEST_WS_PORT)
	sioc.begin(WEBSOCKET_URL, TEST_WS_PORT);
#else
	sioc.begin(WEBSOCKET_URL, WEBSOCKET_PORT);
#endif
}

void loop() {
	sioc.loop();

	switch(currentGame) {
		case GAME_VIEROPEENRIJ:
			if (!currentGameSetup) {
				gameVierOpEenRij.setSocket(&sioc);
				currentGameSetup = true;
				gameVierOpEenRij.setup();
			}
			gameVierOpEenRij.loop();
			break;
		case GAME_THIRTYSECONDS:
			if (!currentGameSetup) {
				gameThirtySeconds.setSocket(&sioc);
				currentGameSetup = true;
				gameThirtySeconds.setup();
			}
			gameThirtySeconds.loop();
			break;
		case GAME_TRIVIALPURSUIT:
			if (!currentGameSetup) {
				gameTrivialPursuit.setSocket(&sioc);
				currentGameSetup = true;
				gameTrivialPursuit.setup();
			}
			gameTrivialPursuit.loop();
			break;
		case GAME_VLOTTEGEESTEN:
			if (!currentGameSetup) {
				gameVlotteGeesten.setSocket(&sioc);
				currentGameSetup = true;
				gameVlotteGeesten.setup();
			}
			gameVlotteGeesten.loop();
			break;
		case GAME_GANZENBORD:
			if (!currentGameSetup) {
				gameGanzenbord.setSocket(&sioc);
				currentGameSetup = true;
				gameGanzenbord.setup();
			}
			gameGanzenbord.loop();
			break;
	}
}