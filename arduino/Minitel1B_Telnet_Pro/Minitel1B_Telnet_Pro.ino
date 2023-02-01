#include <Minitel1B_Hard.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebSocketsClient.h> // src: https://github.com/Links2004/arduinoWebSockets.git

#define MINITEL_PORT Serial2

#define DEBUG true
#define DEBUG_PORT Serial

#if DEBUG // Debug enabled
#define debugBegin(x)     DEBUG_PORT.begin(x)
#define debugPrint(x)     DEBUG_PORT.print(x)
#define debugPrintln(x)   DEBUG_PORT.println(x)
#define debugPrintf(...)    DEBUG_PORT.printf(__VA_ARGS__)
//#define debugPrintf(x,y)  DEBUG_PORT.printf(x,y)
#else // Debug disabled : Empty macro functions
#define debugBegin(x)
#define debugPrint(x)
#define debugPrintln(x)
//#define debugPrintf(x,y)
#define debugPrintf(...)

#endif

Minitel minitel(MINITEL_PORT);

WiFiClient telnet;
Preferences prefs;
WebSocketsClient webSocket;

// WiFi credentials
String ssid("");
String password("");

String url("");
String host("");
String path("");
uint16_t port = 0;
bool scroll = true;
bool echo = false;
bool col80 = false;
int ping_ms = 0;
String protocol("");

byte connectionType = 0; // 0=Telnet 1=Websocket
bool ssl = false;

/****** TELETEL ------ connecté le 9 déc 2021
  String host = "home.teletel.org";
  uint16_t port = 9000;
  bool col80 = false; // mode mixte
  bool scroll = false; // mode rouleau
  bool echo = false; // local echo
  /**/

/*
  String host = "bbs.retrocampus.com";
  uint16_t port = 6503; // Apple-1 without echo
  //const char* host = "172.16.100.210";
  //
  bool col80 = false; // mode mixte
  bool scroll = true; // mode rouleau
  bool echo = false; // local echo
*/


/****** GLASSTTY - TELSTAR ------ connecté le 22 juin 2022
  // https://glasstty.com/using-minitel-terminals-with-telstar/
  // changer les bauds à 1200 bauds (lignes 64 & 65),
  // appuyer CONNEXIONFIN pour que le minitel soit détecté
  // puis faire *# pour recharger la page
  String host = "glasstty.com";
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

  bool connectionOk = true;
  do {
    minitel.extendedKeyboard();
    minitel.textMode();
    minitel.clearScreen();
    minitel.modeVideotex();
    minitel.textMode();
    minitel.echo(false);
    minitel.pageMode();

    loadPrefs();
    debugPrintln("Prefs loaded");

    minitel.clearScreen();
    showPrefs();
    setPrefs();
  
    separateUrl(url);

    minitel.println("Connecting, please wait. CTRL+R to reset");

    // WiFi connection
    debugPrintf("\nWiFi Connecting to %s ", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      debugPrint(".");
      unsigned long key = minitel.getKeyCode();
      if (key == 18) { // CTRL+R = RESET
        minitel.clearScreen();
        minitel.moveCursorXY(1, 1);
        WiFi.disconnect();
        ESP.restart();
      }
    }
    debugPrintln();
    debugPrint("WiFi connected with local IP: ");
    debugPrintln(WiFi.localIP());
    minitel.print("Connected with IP ");
    minitel.println(WiFi.localIP().toString());


    if (connectionType == 0) { // TELNET --------------------------------------------------------------------------------------
      // Telnet server connection
      delay(100);
      debugPrintf("Connecting to %s\n", host.c_str());
      minitel.print("Connecting to "); minitel.print(host); minitel.print(":"); minitel.println(String(port));
  
      if (telnet.connect(host.c_str(), port)) {
        debugPrintln("Connected");
      } else {
        debugPrintln("Connection failed");
        minitel.println();
        minitel.println("Connection Refused. Press any key");
        while (minitel.getKeyCode() == 0);
        connectionOk = false;
      }

      
    } else if (connectionType == 1) { // WEBSOCKET -----------------------------------------------------------------------------
      delay(100);
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

    }  // --------------------------------------------------------------------------------------------------------------------------
  
  
  
  } while (!connectionOk);

  minitel.textMode();
  minitel.clearScreen();

  // Set 40 or 80 columns
  if (col80) {
    minitel.modeMixte();
  } else {
    minitel.modeVideotex();
    minitel.textMode();
  }

  // Set echo
  minitel.echo(echo);

  // Set scroll
  if (scroll) {
    minitel.scrollMode();
  } else {
    minitel.pageMode();
  }

  minitel.clearScreen();
  minitel.moveCursorXY(1, 1);

  debugPrintln("Minitel initialized");

}

void loop() {
  if (connectionType == 0) // TELNET
    loopTelnet();
  else if (connectionType == 1) // WEBSOCKET
    loopWebsocket();
}

void loopTelnet() {

  if (telnet.available()) {
    int tmp = telnet.read();
    minitel.writeByte((byte) tmp);
    debugPrintf("[telnet] %x\n", tmp);
  }

  if (MINITEL_PORT.available() > 0) {
    byte tmp = minitel.readByte();
    if (tmp == 18) { // CTRL+R = RESET
      telnet.stop();
      WiFi.disconnect();
      minitel.modeVideotex();
      minitel.clearScreen();
      minitel.moveCursorXY(1, 1);
      minitel.echo(true);
      minitel.pageMode();
      ESP.restart();
    }
    telnet.write((uint8_t) tmp);
    debugPrintf("[keyboard] %x\n", tmp);
  }

}

String inputString(String defaultValue, int& exitCode) {
  return inputString(defaultValue, exitCode, ' ');
}

String inputString(String defaultValue, int& exitCode, char padChar) {
  String out = defaultValue == NULL ? "" : defaultValue;
  minitel.print(out);
  minitel.cursor();
  unsigned long key = minitel.getKeyCode();
  while (!(
           key == 4929 || // Invio
           key == 13   ||
           key == 10   ||
           key == 27   || // ESC
           key == 3       // CTRL+C
         )) {
    if (key != 0) {
      Serial.printf("Key = %u\n", key);
      if (key >= 32 && key <= 127) {
        out.concat((char)key);
        minitel.printChar(key);
      } else if (out.length() > 0 && (key == 8 || key == 4935)) { // BACKSPACE
        out.remove(out.length() - 1);
        minitel.noCursor();
        minitel.moveCursorLeft(1);
        minitel.printChar(padChar);
        minitel.moveCursorLeft(1);
        minitel.cursor();
      } else if (key == 18) { // CTRL+R = RESET
        minitel.modeVideotex();
        minitel.clearScreen();
        minitel.moveCursorXY(1, 1);
        minitel.echo(true);
        minitel.pageMode();
        ESP.restart();
      } else if (key == 4933) { // ANNUL
        minitel.noCursor();
        for (int i=0; i<out.length(); ++i) {
          minitel.moveCursorLeft(1);
        }
        for (int i=0; i<out.length(); ++i) {
          minitel.printChar(padChar);
        }
        for (int i=0; i<out.length(); ++i) {
          minitel.moveCursorLeft(1);
        }
        out = "";
        minitel.cursor();
      }
    }
    key = minitel.getKeyCode();
  }
  if (key == 3 || key == 27)
    exitCode = 1;
  else
    exitCode = 0;
  minitel.noCursor();
  minitel.println();
  return out;
}

void loadPrefs() {
  prefs.begin("telnet-pro", true);
  debugPrintln("freeEntries = " + String(prefs.freeEntries()));
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  url = prefs.getString("url", "");
  scroll = prefs.getBool("scroll", false);
  echo = prefs.getBool("echo", false);
  col80 = prefs.getBool("col80", false);
  connectionType = prefs.getUChar("connectionType", 0);
  ping_ms = prefs.getInt("ping_ms", 0);
  protocol = prefs.getString("protocol", "");
  prefs.end();
}

void savePrefs() {
  prefs.begin("telnet-pro", false);
  if (prefs.getString("ssid",     "") != ssid)     prefs.putString("ssid", ssid);
  if (prefs.getString("password", "") != password) prefs.putString("password", password);
  if (prefs.getString("url",      "") != url)     prefs.putString("url", url);
  if (prefs.getBool("scroll", false) != scroll) prefs.putBool("scroll", scroll);
  if (prefs.getBool("echo",   false) != echo)   prefs.putBool("echo",   echo);
  if (prefs.getBool("col80",  false) != col80)  prefs.putBool("col80",  col80);
  if (prefs.getUChar("connectionType", 0) != connectionType) prefs.putUChar("connectionType", connectionType);
  if (prefs.getInt("ping_ms", 0) != ping_ms) prefs.putInt("ping_ms", ping_ms);
  if (prefs.getString("protocol", "") != protocol) prefs.putString("protocol", protocol);
  prefs.end();
}

void showPrefs() {
  minitel.noCursor();
  minitel.attributs(GRANDEUR_NORMALE); minitel.attributs(CARACTERE_BLANC); minitel.attributs(FOND_NOIR);
  minitel.newXY(1,0); minitel.cancel(); minitel.moveCursorDown(1);
  minitel.moveCursorXY(9, 1);
  minitel.attributs(FIN_LIGNAGE);
  minitel.attributs(DOUBLE_HAUTEUR); minitel.attributs(CARACTERE_JAUNE); minitel.attributs(INVERSION_FOND); minitel.println("  Minitel Telnet Pro  ");
  minitel.attributs(FOND_NORMAL); minitel.attributs(GRANDEUR_NORMALE);
  minitel.moveCursorXY(1,4);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("1"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("SSID: "); minitel.attributs(CARACTERE_CYAN); printStringValue(ssid); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("2"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Pass: "); minitel.attributs(CARACTERE_CYAN); printPassword(); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,7);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("3"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("URL: "); minitel.attributs(CARACTERE_CYAN); printStringValue(url); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,9);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("4"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Scroll: "); writeBool(scroll); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("5"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Echo  : "); writeBool(echo); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("6"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Col80 : "); writeBool(col80); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,13);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("7"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Type  : "); writeConnectionType(connectionType); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("8"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("PingMS: "); minitel.attributs(CARACTERE_CYAN); minitel.print(String(ping_ms)); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("9"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Prot. : "); minitel.attributs(CARACTERE_CYAN); minitel.print(protocol); minitel.clearLineFromCursor(); minitel.println();

/*
  minitel.moveCursorXY(16, 15);
  minitel.attributs(DOUBLE_LARGEUR); minitel.attributs(CARACTERE_MAGENTA); // minitel.attributs(CLIGNOTEMENT);
  minitel.print("PRESS");
  minitel.moveCursorXY(16, 16);
  minitel.attributs(DOUBLE_GRANDEUR);
  minitel.print("SPACE");
  minitel.moveCursorXY(10, 18);
  minitel.attributs(DOUBLE_LARGEUR);
  minitel.print("TO CONNECT");
  minitel.attributs(GRANDEUR_NORMALE); minitel.attributs(CARACTERE_BLANC); //minitel.attributs(FIXE);
*/

  minitel.attributs(CARACTERE_JAUNE); minitel.moveCursorXY(9,22); minitel.print("use CTRL+R to restart");

  minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_BLEU); minitel.print("(C) 2023 Louis H - Francesco Sblendorio");
  minitel.attributs(CARACTERE_BLANC);
}

void printPassword() {
  if (password == NULL || password == "") {
    minitel.print("-undefined-");
  } else {
    minitel.graphicMode();
    minitel.attributs(DEBUT_LIGNAGE);
    for (int i = 0; i < password.length(); ++i) minitel.graphic(0b001100);
    minitel.attributs(FIN_LIGNAGE);
    minitel.textMode();
  }
}

void printStringValue(String s) {
  if (s == NULL || s == "") {
    minitel.print("-undefined-");
  } else {
    minitel.print(s);
  }
}


void setPrefs() {
  unsigned long key = minitel.getKeyCode();
  bool valid = false;
  while (key != 32) {
    valid = false;
    if (key != 0) {
      valid = true;
      Serial.printf("Key = %u\n", key);
      if (key == 18) { // CTRL+R = RESET
        valid = false;
        minitel.modeVideotex();
        minitel.clearScreen();
        minitel.moveCursorXY(1, 1);
        minitel.echo(true);
        minitel.pageMode();
        ESP.restart();
      } else if (key == '1') {
        setParameter(10, 4, ssid, false, false);
      } else if (key == '2') {
        setParameter(10, 5, password, true, false);
      } else if (key == '3') {
        setParameter(9, 7, url, false, false);
      } else if (key == '4') {
        switchParameter(12, 9, scroll);
      } else if (key == '5') {
        switchParameter(12, 10, echo);
      } else if (key == '6') {
        switchParameter(12, 11, col80);
      } else if (key == '7') {
        cycleConnectionType();
      } else if (key == '8') {
        uint16_t temp = ping_ms;
        setIntParameter(12, 14, temp);
        ping_ms = temp;
      } else if (key == '9') {
        setParameter(12, 15, protocol, false, true);
      } else {
        valid = false;
      }
    }
    if (valid) {
      savePrefs();
    }
    key = minitel.getKeyCode();
  }
  minitel.clearScreen();
  minitel.moveCursorXY(1, 1);
}

void cycleConnectionType() {
  connectionType = (connectionType + 1) % 2;
  minitel.moveCursorXY(12,13); writeConnectionType(connectionType);
}

void switchParameter(int x, int y, bool &destination) {
  destination = !destination;
  minitel.moveCursorXY(x, y); writeBool(destination);
}

void setParameter(int x, int y, String &destination, bool mask, bool allowBlank) {
  minitel.moveCursorXY(x, y); minitel.attributs(CARACTERE_BLANC);
  minitel.print(destination);
  Serial.printf("************ %d ***********\n", 41 - x - destination.length());
  int len = 41 - x - destination.length();
  if (len <= 0) len = 0;
  for (int i = 0; i < len; ++i) minitel.print(".");
  minitel.moveCursorXY(x, y);
  int exitCode = 0;
  String temp = inputString(destination, exitCode, '.');
  if (!exitCode) {
    if (allowBlank) {
      destination = String(temp);
    } else if (temp.length() > 0) {
      destination = String(temp);
    }
  }
  minitel.moveCursorXY(x, y); minitel.attributs(CARACTERE_CYAN);
  if (destination == "") {
    if (!allowBlank) minitel.print("-undefined-");
  } else {
    if (mask) {
      minitel.graphicMode();
      minitel.attributs(DEBUT_LIGNAGE);
      for (int i = 0; i < destination.length(); ++i) minitel.graphic(0b001100);
      minitel.attributs(FIN_LIGNAGE);
      minitel.textMode();
    } else
      printStringValue(destination);
  }
  minitel.clearLineFromCursor();
}

void setIntParameter(int x, int y, uint16_t &destination) {
  String strParam = String(destination);
  if (strParam == "0") strParam = "";
  minitel.moveCursorXY(x, y); minitel.attributs(CARACTERE_BLANC);
  minitel.print(strParam);
  for (int i = 0; i < 41 - x - String(destination).length(); ++i) minitel.print(".");
  minitel.moveCursorXY(x, y);
  int exitCode = 0;
  String temp = inputString(strParam, exitCode, '.');
  if (!exitCode && temp.length() > 0) {
    destination = temp.toInt();
  }
  minitel.moveCursorXY(x, y); minitel.attributs(CARACTERE_CYAN);
  minitel.print(String(destination));
  minitel.clearLineFromCursor();
}

void writeBool(bool value) {
  if (value) {
    minitel.attributs(CARACTERE_VERT); minitel.print("Yes");
  } else {
    minitel.attributs(CARACTERE_ROUGE); minitel.print("No ");
  }
  minitel.attributs(CARACTERE_BLANC);
}

void writeConnectionType(byte connectionType) {
  if (connectionType == 0) {
    minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
  } else {
    minitel.attributs(CARACTERE_BLEU); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("Telnet");
  minitel.attributs(CARACTERE_BLEU); minitel.attributs(FOND_NORMAL); minitel.print("/");

  if (connectionType == 1) {
    minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
  } else {
    minitel.attributs(CARACTERE_BLEU); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("Websocket");
/*  
  minitel.attributs(CARACTERE_BLEU); minitel.attributs(FOND_NORMAL); minitel.print("/");

  if (connectionType == 2) {
    minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
  } else {
    minitel.attributs(CARACTERE_BLEU); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("SSH");
*/

  minitel.attributs(CARACTERE_BLANC); minitel.attributs(FOND_NORMAL);
}

void separateUrl(String url) {

  url.trim();
  String temp = String(url);
  temp.toLowerCase();

  if (temp.startsWith("wss://")) {
    ssl = true;
    url.remove(0, 6);
  } else if (temp.startsWith("ws://")) {
    ssl = false;
    url.remove(0, 5);
  } else if (temp.startsWith("wss:")) {
    ssl = true;
    url.remove(0, 4);
  } else if (temp.startsWith("ws:")) {
    ssl = false;
    url.remove(0, 3);
  } else {
    ssl = false;
  }

  int colon = url.indexOf(':');
  int slash = url.indexOf('/', colon);

  if (slash == -1 && colon == -1) {
    host = url;
    port = 0;
    path = "/";
  } else if (slash == -1 && colon != -1) {
    host = url.substring(0, colon);
    port = url.substring(colon+1).toInt();
    path = "/";
  } else if (slash != -1 && colon == -1) {
    host = url.substring(0, slash);
    port = 0;
    path = url.substring(slash+1);
  } else if (slash != -1 && colon != -1) {
    host = url.substring(0, colon);
    port = url.substring(colon+1, slash).toInt();
    path = url.substring(slash);
  }

  if (port == 0) {
    if (connectionType == 0) {
      port = 23;
    } else if (connectionType == 1) {
      port = 80;
    } else if (connectionType == 2) {
      port = 22;
    }
  }
}

void loopWebsocket() {

  // Websocket -> Minitel
  webSocket.loop();

  // Minitel -> Websocket
  uint32_t key = minitel.getKeyCode(false);
  if (key != 0) {
    if (key == 18) { // CTRL + R = RESET
      webSocket.disconnect();
      WiFi.disconnect();
      minitel.modeVideotex();
      minitel.clearScreen();
      minitel.moveCursorXY(1, 1);
      minitel.echo(true);
      minitel.pageMode();
      ESP.restart();
    }
    debugPrintf("[KB] got code: %X\n", key);
    // prepare data to send over websocket
    uint8_t payload[4];
    size_t len = 0;
    for (len = 0; key != 0 && len < 4; len++) {
      payload[3-len] = uint8_t(key);
      key = key >> 8;
    }
    webSocket.sendBIN(payload+4-len, len);
  }

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t len) {
  switch(type) {
    case WStype_DISCONNECTED:
      debugPrintf("[WS] Disconnected!\n");
      minitel.println();
      minitel.println("DISCONNECTING...");
      delay(3000);
      webSocket.disconnect();
      WiFi.disconnect();
      ESP.restart();
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
      debugPrintf("[WS] got %u binaries\n", len);
      if (len > 0) {
        debugPrintf("  >  %s\n", payload);
        for (size_t i = 0; i < len; i++) {
          minitel.writeByte(payload[i]);
        }
      }
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
