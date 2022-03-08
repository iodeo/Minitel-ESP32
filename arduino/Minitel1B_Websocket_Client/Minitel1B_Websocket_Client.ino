/*
 * Sample code for connexion to minitel videotex server via websocket 
 * Requirements: ESP32 connected to minitel DIN port and a WiFi connexion
 * 
 * created by iodeo - dec 2021
 */

#include <WiFi.h>
#include <WebSocketsClient.h> // src: https://github.com/Links2004/arduinoWebSockets.git
#include <Minitel1B_Hard.h>   // src: https://github.com/eserandour/Minitel1B_Hard.git

// ---------------------------------------
// ------ Minitel port configuration

#define MINITEL_PORT Serial2       // for Minitel-ESP32 devboard
#define MINITEL_BAUD 4800          // 1200 / 4800 / 9600 depending on minitel type
#define MINITEL_DISABLE_ECHO true  // true if characters are repeated when typing

// ---------------------------------------
// ------ Debug port configuration

#define DEBUG true

#if DEBUG
  #define DEBUG_PORT Serial       // for Minitel-ESP32 devboard
  #define DEBUG_BAUD 115200       // set serial monitor accordingly
  #define debugBegin(x)     DEBUG_PORT.begin(x)
  #define debugPrintf(...)    DEBUG_PORT.printf(__VA_ARGS__)
#else // Empty macro functions
  #define debugBegin(x)     
  #define debugPrintf(...)
#endif

// ---------------------------------------
// ------ WiFi credentials

const char* ssid     = "mySsid";        // your wifi network
const char* password = "myPassword";    // your wifi password

// ---------------------------------------
// ------ Websocket server

/******  TELETEL.ORG  ---------  connecté le 2 mar 2022*/
// ws://home.teletel.org:9001/
char* host = "home.teletel.org";
int port = 9001;
char* path = "/";
bool ssl = false;
int ping_ms = 0;
char* protocol = "";
/**/

/****** 3615  -----------------  connecté le 2 mar 2022
// wss://3615co.de/ws
char* host = "3615co.de";
int port = 80;
char* path = "/ws"; 
bool ssl = false;
int ping_ms = 0;
char* protocol = "";
/**/

/****** AE  -------------------  connecté le 2 mar 2022
// ws://3611.re/ws
// websocket payload length of 0
char* host = "3611.re";
int port = 80;
char* path = "/ws";
bool ssl = false;
int ping_ms = 0;
char* protocol = "";
/**/

/******  HACKER  --------------  connecté le 2 mar 2022
// ws://mntl.joher.com:2018/?echo
// websocket payload length up to 873
char* host = "mntl.joher.com";
int port = 2018;
char* path = "/?echo";
bool ssl = false;
int ping_ms = 0;
char* protocol = "";
/**/

/****** TEASER  ---------------  connecté le 2 mar 2022
// ws://minitel.3614teaser.fr:8080/ws
char* host = "minitel.3614teaser.fr";
int port = 8080;
char* path = "/ws"; 
bool ssl = false;
int ping_ms = 10000;
char* protocol = "tty";
/**/

/****** SM  -------------------  connecté le 2 mar 2022
// wss://wss.3615.live:9991/?echo
// websocket payload length up to 128
char* host = "wss.3615.live";
int port = 9991;
char* path = "/?echo"; 
bool ssl = true;
int ping_ms = 0;
char* protocol = "";
/**/

WiFiClient client;
WebSocketsClient webSocket;
Minitel minitel(MINITEL_PORT);

void setup() {

  debugBegin(DEBUG_BAUD);
  debugPrintf("\n-----------------------\n");
  debugPrintf("\n> Debug port ready\n");

  // We initiate minitel communication
  debugPrintf("\n> Minitel setup\n");
  int baud = minitel.searchSpeed();
  if (baud != MINITEL_BAUD) baud = minitel.changeSpeed(MINITEL_BAUD);
  debugPrintf("  - Baud detected: %u\n", baud);
  if (MINITEL_DISABLE_ECHO) {
    minitel.echo(false);
    debugPrintf("  - Echo disabled\n");
  }

  // We connect to WiFi network
  debugPrintf("\n> Wifi setup\n");
  debugPrintf("  Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debugPrintf(".");
  }
  debugPrintf("\n  WiFi connected with IP %s\n", WiFi.localIP().toString().c_str());

  // We connect to Websocket server
  debugPrintf("\n> Websocket connection\n");
  if (protocol[0] == '\0') {
    if (ssl) webSocket.beginSSL(host, port, path);
    else webSocket.begin(host, port, path);
  }
  else {
    debugPrintf("  - subprotocol added\n");
    if (ssl) webSocket.beginSSL(host, port, path, protocol);
    else webSocket.begin(host, port, path, protocol);
  }
  
  webSocket.onEvent(webSocketEvent);
  
  if (ping_ms != 0) {
    debugPrintf("  - heartbeat ping added\n");
    // start heartbeat (optional)
    // ping server every ping_ms
    // expect pong from server within 3000 ms
    // consider connection disconnected if pong is not received 2 times
    webSocket.enableHeartbeat(ping_ms, 3000, 2);
  }

  debugPrintf("\n> End of setup\n\n");

}

void loop() {

  // Websocket -> Minitel
  webSocket.loop();

  // Minitel -> Websocket
  uint32_t key = minitel.getKeyCode(false);
  if (key != 0) {
    debugPrintf("[KB] got code: %X\n", key);
    // prepare data to send over websocket
    uint8_t payload[4];
    size_t len = 0;
    for (len = 0; key != 0 && len < 4; len++) {
      payload[3-len] = uint8_t(key);
      key = key >> 8;
    }
    webSocket.sendTXT(payload+4-len, len);
  }

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t len) {
  switch(type) {
    case WStype_DISCONNECTED:
      debugPrintf("[WS] Disconnected!\n");
      break;
      
    case WStype_CONNECTED:
      debugPrintf("[WS] Connected to url: %s\n", payload);
      break;
      
    case WStype_TEXT:
      debugPrintf("[WS] got %u chars\n", len);
      if (len > 0) {
        debugPrintf("  >  %s\n", payload);
        for (size_t i = 0; i < len; i++) {
          minitel.writeByte(payload[i]);
        }
      }
      break;
      
    case WStype_BIN:
      debugPrintf("[WS] got %u binaries - ignored\n", len);
      break;
      
    case WStype_ERROR:
      debugPrintf("[WS] WStype_ERROR\n");
      break;
      
    case WStype_FRAGMENT_TEXT_START:
      debugPrintf("[WS] WStype_FRAGMENT_TEXT_START\n");
      break;
      
    case WStype_FRAGMENT_BIN_START:
      debugPrintf("[WS] WStype_FRAGMENT_BIN_START\n");
      break;
      
    case WStype_FRAGMENT:
      debugPrintf("[WS] WStype_FRAGMENT\n");
      break;
      
    case WStype_FRAGMENT_FIN:
      debugPrintf("[WS] WStype_FRAGMENT_FIN\n");
      break;
  }
}
