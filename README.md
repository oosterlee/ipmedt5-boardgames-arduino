# ipmedt5-boardgames-arduino

## configuration
file: `config.h`
```c++
#define TEST_GAME GAME_VIEROPEENRIJ // Game to test one of: [GAME_VIEROPEENRIJ, GAME_THIRTYSECONDS, GAME_TRIVIALPURSUIT, GAME_VLOTTEGEESTEN, GAME_GANZENBORD]
#define TEST_WS_URL "" // IP address of computer here (eg. 192.168.1.123)
#define TEST_WS_PORT 8000
```

### Maak **GEEN** gebruik van `delay()`
##### Voorbeeld met `delay()`
```c++
void loop() {
	digitalWrite(PIN, !digitalRead(PIN));
	delay(1000);
}
```
##### Voorbeeld zonder `delay()`
```c++
long lastBlink = 0;
long delayTime = 1000;

void loop() {
	now = millis();
	if (now >= lastBlink+delayTime) {
		lastBlink = now;
		digitalWrite(PIN, !digitalRead(PIN));
	}
}
```
