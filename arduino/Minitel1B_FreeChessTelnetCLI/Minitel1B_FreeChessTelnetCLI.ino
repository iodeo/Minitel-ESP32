#include <Minitel1B_Hard.h>
#include "src/Telnet_Client.h"

#define MINITEL_PORT Serial2 //for ESP32

#define DEBUG true
#define DEBUG_PORT Serial
 
#if DEBUG // Debug enabled
  #define debugBegin(x)     DEBUG_PORT.begin(x)
  #define debugPrint(x)     DEBUG_PORT.println(x)
  #define debugPrintHEX(x)  DEBUG_PORT.println(x,HEX)
  #define debugPrintBIN(x)  DEBUG_PORT.println(x,BIN)
#else // Debug disabled : Empty macro functions
  #define debugBegin(x)     
  #define debugPrint(x)     
  #define debugPrintHEX(x)  
  #define debugPrintBIN(x)  
#endif

Minitel minitel(MINITEL_PORT);

TelnetClient telnet;
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASS";
//IPAddress freechessIP (167, 114, 65, 195); 
const char* host = "freechess.org"
uint16_t port = 5000;

void setup() {
  
  debugBegin(115200);
  debugPrint();
  debugPrint("Debug ready");

  // Minitel setup
  if (minitel.searchSpeed() != 4800) {     // search speed
    if (minitel.changeSpeed(4800) < 0) {   // set to 4800 if different
      minitel.searchSpeed();               // search speed again if change has failed
    }
  }
  debugPrint("minitel speed set");
  
  minitel.modeMixte();
  minitel.echo(false);  
  minitel.standardKeyboard(); //block arrows in editing mode  
  minitel.smallMode(); //uncap
  minitel.clearScreen();
  minitel.moveCursorXY(1,1);
  debugPrint("Done");

  // WiFi connection 
  debugPrint();
  debugPrint("WiFi Connecting to " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  debugPrint("");
  debugPrint("WiFi connected");
  debugPrint("IP address: ");
  debugPrint(WiFi.localIP());

  // Host connection
  delay(100);
  debugPrint("Connecting to " + String(host));
  if (telnet.connect(host, port)) debugPrint("Connected");
  else debugPrint("Connection failed");
}

String telnetInput = "";
String keyboardInput = "";
bool receiving = false;

void loop() {

  char c = 0;

  if (telnet.available()) {
    receiving = true;
    c = telnet.readChar();
    if (c == '|') c = '!';
    telnetInput += c;
  }
  else { // print back keyboard input after telenet reception
    if (receiving) {
      receiving = false;
      minitel.print(keyboardInput);
    }
  }

  if (c == '\n') {
    debugPrint("[TELNET INPUT] " + String(telnetInput));
    minitel.println(telnetInput);
    telnetInput = "";
  }

  c = getKeyboardChar();
  if (c != 0) editKeyboardInput(c);
  
  if (c == '\r') {
    debugPrint("[KEYBOARD INPUT] " + String(keyboardInput)); 
    telnet.println(keyboardInput);
    minitel.moveCursorReturn(1);
    keyboardInput = "";
  }
  
}

char getKeyboardChar() {
  
  unsigned long key = minitel.getKeyCode();

  if ((key != CONNEXION_FIN) &&
      (key != SOMMAIRE) &&
      (key != ANNULATION) &&
      (key != RETOUR) &&
      (key != REPETITION) &&
      (key != GUIDE) &&
      (key != CORRECTION) &&
      (key != SUITE) &&
      (key != ENVOI)) {
    return key;
  }
  else {
    // key redirections
    switch (key) {
      case CORRECTION:  return DEL;  break; // delete
      case ANNULATION:  return CAN;  break; // cancel
      case ENVOI:       return CR;   break; // carriage return
    }
  }
}

void editKeyboardInput(char c) {
  
  switch (c) {
    
    case CAN:
      //debugPrint("CAN");
      keyboardInput = "";
      minitel.deleteLines(1);
      break;
      
    case DEL:
      //debugPrint("DEL");
      keyboardInput.remove(keyboardInput.length() - 1);
      minitel.moveCursorLeft(1);
      minitel.deleteChars(1);
      break;
      
    default:
      //debugPrint("DEFAUT");
      keyboardInput += c;
      minitel.printChar(c); //echo to minitel screen
  }
  /*if (c != 0) {
    debugPrint(c,HEX);
    debugPrint(" > ");
    debugPrint(keyboardInput);
  }*/
}
