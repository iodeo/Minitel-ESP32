#include <Minitel1B_Hard.h>
#include <WiFi.h>

#define MINITEL_PORT Serial2

#define DEBUG true
#define DEBUG_PORT Serial
 
#if DEBUG // Debug enabled
  #define debugBegin(x)     DEBUG_PORT.begin(x)
  #define debugPrint(x)     DEBUG_PORT.print(x)
  #define debugPrintln(x)   DEBUG_PORT.println(x)
  #define debugPrintf(x,y)  DEBUG_PORT.printf(x,y)
#else // Debug disabled : Empty macro functions
  #define debugBegin(x)     
  #define debugPrint(x)     
  #define debugPrintln(x)   
  #define debugPrintf(x,y)   
#endif

Minitel minitel(MINITEL_PORT);

WiFiClient telnet;

// WiFi credentials
const char* ssid     = "TecnoScientia";
const char* password = "babylon5";

// Telnet server

/****** TELETEL ------ connecté le 9 déc 2021
const char* host = "home.teletel.org";
uint16_t port = 9000;
bool col80 = false; // mode mixte
bool scroll = false; // mode rouleau
bool echo = false; // local echo
/**/

/*

const char* host = "bbs.retrocampus.com";
uint16_t port = 6503; // Apple-1 without echo
//const char* host = "172.16.100.210";
//
bool col80 = false; // mode mixte
bool scroll = true; // mode rouleau
bool echo = false; // local echo

*/


 
const char* host = "178.79.152.19";
uint16_t port = 3615; // Apple-1 without echo
bool col80 = false; // mode mixte
bool scroll = false; // mode rouleau
bool echo = false; // local echo

/****** GLASSTTY - TELSTAR ------ connecté le 22 juin 2022
// https://glasstty.com/using-minitel-terminals-with-telstar/
// changer les bauds à 1200 bauds (lignes 64 & 65), 
// appuyer CONNEXIONFIN pour que le minitel soit détecté
// puis faire *# pour recharger la page
const char* host = "glasstty.com";
uint16_t port = 6502; // 
bool col80 = false; // mode mixte
bool scroll = false; // mode rouleau
bool echo = false; // local echo
/**/

void setup() {
  
  debugBegin(115200);
  debugPrintln("----------------");
  debugPrintln("Debug ready");

  // Minitel setup
  if (minitel.searchSpeed() != 4800) {     // search speed
    if (minitel.changeSpeed(4800) < 0) {   // set to 4800 if different
      minitel.searchSpeed();               // search speed again if change has failed
    }
  }
  debugPrintln("Minitel baud set");

  minitel.clearScreen();
  if (col80) minitel.modeMixte();
  else minitel.modeVideotex();
  minitel.echo(echo);
  if (scroll) minitel.scrollMode();
  else minitel.pageMode();

  debugPrintln("Minitel initialized");

  // WiFi connection 
  debugPrintf("\nWiFi Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      debugPrint(".");
  }
  debugPrintln();
  debugPrint("WiFi connected with local IP: ");
  debugPrintln(WiFi.localIP());

  // Telnet server connection
  delay(100);
  debugPrintf("Connecting to %s\n", host);
  if (telnet.connect(host, port)) debugPrintln("Connected");
  else debugPrintln("Connection failed");
}

void loop() {

  if (telnet.available()) {
    int tmp = telnet.read();
    minitel.writeByte((byte) tmp);
    debugPrintf("[telnet] %x\n", tmp);
  }

  if (MINITEL_PORT.available() > 0) {
    byte tmp = minitel.readByte();
    telnet.write((uint8_t) tmp);
    debugPrintf("[keyboard] %x\n", tmp);
  }
  
}
