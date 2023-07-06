#include <Minitel1B_Hard.h>
#include <Preferences.h>
#include <WiFi.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <WebSocketsClient.h> // src: https://github.com/Links2004/arduinoWebSockets.git
#include "sshClient.h"

#define MINITEL_BAUD_TRY  4800

#define MINITEL_PORT Serial2

#define DEBUG true
#define DEBUG_PORT Serial

#if DEBUG // Debug enabled
#define debugBegin(x)     DEBUG_PORT.begin(x)
#define debugPrint(x)     DEBUG_PORT.print(x)
#define debugPrintln(x)   DEBUG_PORT.println(x)
#define debugPrintf(...)    DEBUG_PORT.printf(__VA_ARGS__)
#else // Debug disabled : Empty macro functions
#define debugBegin(x)
#define debugPrint(x)
#define debugPrintln(x)
#define debugPrintf(...)

#endif

Minitel minitel(MINITEL_PORT);

WiFiClient telnet;
Preferences prefs;
WebSocketsClient webSocket;
SSHClient sshClient;
TaskHandle_t sshTaskHandle;

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
String sshUser("");
String sshPass("");

byte connectionType = 0; // 0=Telnet 1=Websocket 2=SSH
bool ssl = false;

typedef struct {
  String presetName = "";
  
  String url = "";
  bool scroll = false;
  bool echo = false;
  bool col80 = false;
  byte connectionType = 0;
  int ping_ms = 0;
  String protocol = "";
  String sshUser = "";
  String sshPass = "";
} Preset;

Preset presets[20];
int speed;

void initFS() {
  boolean ok = SPIFFS.begin();
  if (!ok)
  {
    minitel.attributs(CLIGNOTEMENT);
    minitel.println("Formating SPIFFS, please wait");
    minitel.attributs(FIXE);
    ok = SPIFFS.format();
    if (ok) SPIFFS.begin();
  }
  if (!ok)
  {
    debugPrintf("%% Aborting now. Problem initializing Filesystem. System HALTED\n");
    minitel.println("System HALTED.");
    minitel.println("problem initializing filesystem");
    while (1) delay(5000);
  }
  debugPrintf(
    "%% Mounted SPIFFS used=%d total=%d\r\n", SPIFFS.usedBytes(),
    SPIFFS.totalBytes());
}

void setup() {
  debugBegin(115200);
  debugPrintln("----------------");
  debugPrintln("Debug ready");

  // Minitel setup
  speed = MINITEL_BAUD_TRY;
  MINITEL_PORT.updateBaudRate(speed); // override minitel1b_Hard default speed
  if (speed != minitel.currentSpeed()) { // avoid unwanted characters when restarting
    if ( (speed = minitel.searchSpeed()) < MINITEL_BAUD_TRY) {    // search speed
      if ( (speed = minitel.changeSpeed(MINITEL_BAUD_TRY)) < 0) { // set to MINITEL_BAUD_TRY if different
        speed = minitel.searchSpeed();                         // search speed again if change has failed
      }
    }
  }

  // minitel.changeSpeed(speed);
  debugPrintf("Minitel baud set to %d\n", speed);

  bool connectionOk = true;
  do {
    minitel.modeVideotex();
    minitel.writeByte(0x1b); minitel.writeByte(0x28); minitel.writeByte(0x40); // Standard G0 textmode charset
    minitel.writeByte(0x1b); minitel.writeByte(0x50); // Set black background
    minitel.textMode();
    minitel.moveCursorXY(1,1);
    minitel.extendedKeyboard();
    minitel.textMode();
    minitel.clearScreen();
    minitel.echo(false);
    minitel.pageMode();

    loadPrefs();
    debugPrintln("Prefs loaded");

    readPresets();
    debugPrintln("Presets loaded");

    showPrefs();
    setPrefs();
  
    separateUrl(url);

    minitel.capitalMode();
    minitel.println("Connecting, please wait. CTRL+R to reset");

    // WiFi connection
    debugPrintf("\nWiFi Connecting to %s ", ssid.c_str());
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      debugPrint(".");
      unsigned long key = minitel.getKeyCode();
      if (key == 18) { // CTRL+R = RESET
        minitel.moveCursorXY(1, 1);
        minitel.clearScreen();
        WiFi.disconnect();
        reset();
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
      debugPrintf("ssl=%d, host=%s, port=%d, path=%s, protocol='%s'\n",ssl, host.c_str(), port, path.c_str(), protocol.c_str());
      
      if (protocol == "") {
        if (ssl) webSocket.beginSSL(host.c_str(), port, path.c_str());
        else webSocket.begin(host.c_str(), port, path.c_str());
      }
      else {
        debugPrintf("  - subprotocol added\n");
        if (ssl) webSocket.beginSSL(host.c_str(), port, path.c_str(), protocol.c_str());
        else webSocket.begin(host.c_str(), port, path.c_str(), protocol.c_str());
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
    } else if (connectionType == 2) { // SSH ---------------------------------------------------------------------------------------
      debugPrintf("\n> SSH task setup\n");
      BaseType_t xReturned;
      xReturned = xTaskCreatePinnedToCore(sshTask, "sshTask", 51200, NULL,
        (configMAX_PRIORITIES - 1), &sshTaskHandle, ARDUINO_RUNNING_CORE);
      if (xReturned!=pdPASS) debugPrintf("  > Failed to create task\n");
    }  // --------------------------------------------------------------------------------------------------------------------------
  
  
  
  } while (!connectionOk);

  minitel.textMode();
  minitel.moveCursorXY(1,1);
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

  minitel.moveCursorXY(1, 1);
  minitel.clearScreen();

  debugPrintln("Minitel initialized");

}

void loop() {
  if (connectionType == 0) // TELNET
    loopTelnet();
  else if (connectionType == 1) // WEBSOCKET
    loopWebsocket();
  else if (connectionType == 2) // SSH
    loopSsh();
}

void loopTelnet() {

  if (telnet.available()) {
    int tmp = telnet.read();
    minitel.writeByte((byte) tmp);
    debugPrintf("[telnet] 0x%X\n", tmp);
  }

  if (MINITEL_PORT.available() > 0) {
    byte tmp = minitel.readByte();
    if (tmp == 18) { // CTRL+R = RESET
      telnet.stop();
      WiFi.disconnect();
      minitel.modeVideotex();
      minitel.moveCursorXY(1, 1);
      minitel.clearScreen();
      minitel.echo(true);
      minitel.pageMode();
      reset();
    }
    telnet.write((uint8_t) tmp);
    debugPrintf("[keyboard] 0x%X\n", tmp);
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
      debugPrintf("Key = %u\n", key);
      String str = minitel.getString(key);
      if (str != "") {
        out.concat(str);
        minitel.print(str);
      } else if (out.length() > 0 && (key == 8 || key == 4935)) { // BACKSPACE
        unsigned int index = out.length()-1;
        if (out.charAt(index) >> 7) // utf-8 multibyte pattern
          while ((out.charAt(index) >> 6) != 0b11) index--; // utf-8 first byte pattern
        out.remove(index);
        minitel.noCursor();
        minitel.moveCursorLeft(1);
        minitel.printChar(padChar);
        minitel.moveCursorLeft(1);
        minitel.cursor();
      } else if (key == 18) { // CTRL+R = RESET
        minitel.modeVideotex();
        minitel.moveCursorXY(1, 1);
        minitel.clearScreen();
        minitel.echo(true);
        minitel.pageMode();
        reset();
      } else if (key == 4933) { // ANNUL
        unsigned int length = numberOfChars(out);
        minitel.noCursor();
        for (int i=0; i<length; ++i) {
          minitel.moveCursorLeft(1);
        }
        for (int i=0; i<length; ++i) {
          minitel.printChar(padChar);
        }
        for (int i=0; i<length; ++i) {
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

unsigned int numberOfChars(String str) {
  // number of chars of a string including utf-8 multibyte characters
  unsigned int index = 0;
  unsigned int count = 0;
  while (index<str.length()) {
    byte car = str.charAt(index);
    if (car >> 5 == 0b110) index+=2; //utf-8 2 bytes pattern
    else if (car >> 4 == 0b1110) index+=3; // utf-8 3 bytes pattern
    else index++; //default (1 byte)
    count++;
  }
  return count;
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
  sshUser = prefs.getString("sshUser", "");
  sshPass = prefs.getString("sshPass", "");
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
  if (prefs.getString("sshUser", "")  != sshUser)  prefs.putString("sshUser",  sshUser);
  if (prefs.getString("sshPass", "")  != sshPass)  prefs.putString("sshPass",  sshPass);
  prefs.end();
}

void showPrefs() {
  minitel.textMode(); minitel.noCursor();
  minitel.smallMode();
  minitel.attributs(GRANDEUR_NORMALE); minitel.attributs(CARACTERE_BLANC); minitel.attributs(FOND_NOIR); minitel.noCursor();
  minitel.newXY(1,0); minitel.cancel(); minitel.moveCursorDown(1);
  minitel.clearScreen();
  minitel.moveCursorXY(9, 1);
  minitel.attributs(FIN_LIGNAGE);
  minitel.textMode();
  minitel.attributs(DOUBLE_HAUTEUR); minitel.attributs(CARACTERE_JAUNE); minitel.attributs(INVERSION_FOND); minitel.println("  Minitel Telnet Pro  ");
  minitel.attributs(FOND_NORMAL); minitel.attributs(GRANDEUR_NORMALE);
  minitel.moveCursorXY(34,2); minitel.attributs(CARACTERE_ROUGE); minitel.print(String(speed)); minitel.print("bps");
  minitel.moveCursorXY(1,4);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("1"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("SSID: "); minitel.attributs(CARACTERE_CYAN); printStringValue(ssid); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("2"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Pass: "); minitel.attributs(CARACTERE_CYAN); printPassword(password); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,7);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("3"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("URL: "); minitel.attributs(CARACTERE_CYAN); printStringValue(url); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,9);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("4"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Scroll: "); writeBool(scroll); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("5"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Echo  : "); writeBool(echo); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("6"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Col80 : "); writeBool(col80); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,13);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("7"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Type    : "); writeConnectionType(connectionType); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("8"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("PingMS  : "); minitel.attributs(CARACTERE_CYAN); minitel.print(String(ping_ms)); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("9"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Subprot.: "); minitel.attributs(CARACTERE_CYAN); minitel.print(protocol); minitel.clearLineFromCursor(); minitel.println();
  minitel.moveCursorXY(1,16);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("U"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("SSH User: "); minitel.attributs(CARACTERE_CYAN); minitel.print(sshUser); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("P"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("SSH Pass: "); minitel.attributs(CARACTERE_CYAN); if (sshPass != NULL && sshPass != "") {printPassword(sshPass);} minitel.clearLineFromCursor(); minitel.println();

  minitel.moveCursorXY(2,19); minitel.attributs(CARACTERE_BLANC); minitel.attributs(DOUBLE_GRANDEUR); minitel.print("S");
  minitel.moveCursorXY(6,19); minitel.attributs(DOUBLE_HAUTEUR); minitel.print("Save Preset");
  minitel.rect(1,18,4,21);

  int delta=24;
  minitel.moveCursorXY(2+delta,19); minitel.attributs(CARACTERE_BLANC); minitel.attributs(DOUBLE_GRANDEUR); minitel.print("L");
  minitel.moveCursorXY(6+delta,19); minitel.attributs(DOUBLE_HAUTEUR); minitel.print("Load Preset");
  minitel.rect(1+delta,18,4+delta,21);

  minitel.attributs(GRANDEUR_NORMALE);
  minitel.attributs(CARACTERE_JAUNE); 
  minitel.moveCursorXY(1,22);
  minitel.attributs(INVERSION_FOND); minitel.print(" SPACE "); minitel.attributs(FOND_NORMAL); minitel.print(" to connect   ");
  minitel.attributs(INVERSION_FOND); minitel.print(" CTRL+R "); minitel.attributs(FOND_NORMAL); minitel.print(" to restart");

  minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_ROUGE); minitel.print("(C) 2023 Louis H. - Francesco Sblendorio");
  minitel.attributs(CARACTERE_BLANC);
}

void printPassword(String password) {
  if (password == NULL || password == "") {
    minitel.print("-undefined-");
  } else {
    minitel.graphicMode();
    minitel.attributs(DEBUT_LIGNAGE);
    for (int i = 0; i < numberOfChars(password); ++i) minitel.graphic(i <= 30 ? 0b001100 : 0b000000);
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


int setPrefs() {
  unsigned long key = minitel.getKeyCode();
  bool valid = false;
  while (key != 32) {
    valid = false;
    if (key != 0) {
      valid = true;
      debugPrintf("Key = %u\n", key);
      if (key == 18) { // CTRL+R = RESET
        valid = false;
        minitel.modeVideotex();
        minitel.moveCursorXY(1, 1);
        minitel.clearScreen();
        minitel.echo(true);
        minitel.pageMode();
        reset();
      } else if (key == '1') {
        setParameter(10, 4, ssid, false, false);
      } else if (key == '2') {
        setParameter(10, 5, password, true, false);
        if (password.length() <= 31) {
          minitel.moveCursorXY(1,6);
          minitel.clearLineFromCursor();
        }
      } else if (key == '3') {
        setParameter(9, 7, url, false, false);
        if (url.length() <= 40-9) {
          minitel.moveCursorXY(1, 8);
          minitel.clearLineFromCursor();
        }
      } else if (key == '4') {
        switchParameter(12, 9, scroll);
      } else if (key == '5') {
        switchParameter(12, 10, echo);
      } else if (key == '6') {
        switchParameter(12, 11, col80);
      } else if (key == '7') {
        cycleConnectionType(14,13);
      } else if (key == '8') {
        uint16_t temp = ping_ms;
        setIntParameter(14, 14, temp);
        ping_ms = temp;
      } else if (key == '9') {
        setParameter(14, 15, protocol, false, true);
      } else if (key == 'u' || key == 'U') {
        setParameter(14, 16, sshUser, false, true);
      } else if (key == 'p' || key == 'P') {
        setParameter(14, 17, sshPass, true, true);
      } else if (key == 's' || key == 'S') {
        savePresets();
      } else if (key == 'l' || key == 'L') {
        loadPresets();
      } else {
        valid = false;
      }
    }
    if (valid) {
      savePrefs();
    }
    key = minitel.getKeyCode();
  }
  minitel.moveCursorXY(1, 1);
  minitel.clearScreen();
  return 0;
}

void savePresets() {
  uint32_t key;
  displayPresets("Save to slot");
  do { 
    minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_VERT); minitel.print("  Choose slot, ESC or SUMMARY to go back");
    minitel.smallMode();
    while ((key = minitel.getKeyCode()) == 0);
    if (key == 27 || key == 4933 || key == 4934) {
      break;
    } else if (key == 18) { // CTRL+R = RESET
      reset();
    } else if ( (key|32) >= 'a' && (key|32) <= 'a'+20-1) {
      int slot = (key|32) - 'a';
      debugPrintf("slot = %d\n", slot);
      minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_VERT); minitel.print("      Name your slot, ESC to cancel"); minitel.clearLineFromCursor();
      String presetName(presets[slot].presetName);
      int exitCode = setParameter(3, 4+slot, presetName, false, true);
      if (exitCode) continue;
      if (presetName == "") {
        minitel.attributs(CARACTERE_CYAN);
        minitel.moveCursorXY(3, 4+slot); minitel.print(presets[slot].presetName);
        continue;
      }
      // save preset
      presets[slot].presetName = presetName;
      presets[slot].url = url;
      presets[slot].scroll = scroll;
      presets[slot].echo = echo;
      presets[slot].col80 = col80;
      presets[slot].connectionType = connectionType;
      presets[slot].ping_ms = ping_ms;
      presets[slot].protocol = protocol;
      presets[slot].sshUser = sshUser;
      presets[slot].sshPass = sshPass;
      writePresets();
    }
  } while (true);
  showPrefs();
}

void loadPresets() {
  uint32_t key;
  displayPresets("Load from slot");
  do { 
    minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_VERT); minitel.print("  Choose slot, ESC or SUMMARY to go back");
    minitel.smallMode();
    while ((key = minitel.getKeyCode()) == 0);
    if (key == 27 || key == 4933 || key == 4934) {
      break;
    } else if (key == 18) { // CTRL+R = RESET
      reset();
    } else if ( (key|32) >= 'a' && (key|32) <= 'a'+20-1) {
      int slot = (key|32) - 'a';
      debugPrintf("slot = %d\n", slot);
      if (presets[slot].presetName == "") {
        continue;
      }

      minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
      minitel.moveCursorXY(3, 4+slot); minitel.print(presets[slot].presetName);
      delay(500);

      url = presets[slot].url;
      scroll = presets[slot].scroll;
      echo = presets[slot].echo;
      col80 = presets[slot].col80;
      connectionType = presets[slot].connectionType;
      ping_ms = presets[slot].ping_ms;
      protocol = presets[slot].protocol;
      sshUser = presets[slot].sshUser;
      sshPass = presets[slot].sshPass;

      minitel.attributs(CARACTERE_CYAN); minitel.attributs(FOND_NORMAL);
      minitel.moveCursorXY(3, 4+slot); minitel.print(presets[slot].presetName);

      break;
    }
  } while (true);
  showPrefs();
}

void displayPresets(String title) {
  static char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  minitel.moveCursorXY(1,1);
  minitel.clearScreen();
  minitel.attributs(DOUBLE_HAUTEUR); minitel.attributs(CARACTERE_CYAN); minitel.println(title); minitel.attributs(GRANDEUR_NORMALE);
  minitel.moveCursorXY(1,4);
  for (int i=0; i<20; ++i) {
    minitel.attributs(CARACTERE_BLANC);
    minitel.printChar(alphabet[i]);
    minitel.print(":");
    minitel.attributs(CARACTERE_CYAN);
    minitel.println(presets[i].presetName);
  }
}

void cycleConnectionType(int x, int y) {
  connectionType = (connectionType + 1) % 3;
  minitel.moveCursorXY(x,y); writeConnectionType(connectionType);
}

void switchParameter(int x, int y, bool &destination) {
  destination = !destination;
  minitel.moveCursorXY(x, y); writeBool(destination);
}

int setParameter(int x, int y, String &destination, bool mask, bool allowBlank) {
  minitel.moveCursorXY(x, y); minitel.attributs(CARACTERE_BLANC);
  minitel.print(destination);
  int len = 41 - x - numberOfChars(destination);
  debugPrintf("************ %d ***********\n", len);
  if (len < 0) len = 0;
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
      for (int i = 0; i < numberOfChars(destination); ++i) minitel.graphic(i <= 30 ? 0b001100 : 0b000000);
      minitel.attributs(FIN_LIGNAGE);
      minitel.textMode();
    } else
      printStringValue(destination);
  }
  minitel.clearLineFromCursor();
  return exitCode;
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
    minitel.attributs(CARACTERE_ROUGE); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("Telnet");
  minitel.attributs(CARACTERE_ROUGE); minitel.attributs(FOND_NORMAL); minitel.print("/");

  if (connectionType == 1) {
    minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
  } else {
    minitel.attributs(CARACTERE_ROUGE); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("Websocket");
   
  minitel.attributs(CARACTERE_ROUGE); minitel.attributs(FOND_NORMAL); minitel.print("/");

  if (connectionType == 2) {
    minitel.attributs(CARACTERE_BLANC); minitel.attributs(INVERSION_FOND);
  } else {
    minitel.attributs(CARACTERE_ROUGE); minitel.attributs(FOND_NORMAL);
  }
  minitel.print("SSH");

  minitel.attributs(CARACTERE_BLANC); minitel.attributs(FOND_NORMAL);
}

void separateUrl(String url) {

  url.trim();
  String temp = String(url);
  temp.toLowerCase();

  ssl = false;
  
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
  } else if (temp.startsWith("ssh://")) {
    url.remove(0, 6);
  } else if (temp.startsWith("ssh:")) {
    url.remove(0, 4);
  }
  
  int colon = url.indexOf(':');
  int slash = url.indexOf('/');

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
    path = url.substring(slash);
  } else if (slash != -1 && colon != -1) {
    host = url.substring(0, colon);
    port = url.substring(colon+1, slash).toInt();
    path = url.substring(slash);
  }

  if (port == 0) {
    if (connectionType == 0) {
      port = 23;
    } else if (connectionType == 1) {
      port = ssl ? 443 : 80;
    } else if (connectionType == 2) {
      port = 22;
    }
  }
}

void loopSsh() {
  // do nothing while sshTask runs
  if (eTaskGetState(sshTaskHandle)!=eDeleted) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  } else {// reset otherwise
    reset();
  }
}

void sshTask(void *pvParameters) {
  debugPrintf("\n> SSH task running\n");
  
  // Open ssh session
  debugPrintf("  Connecting to %s as %s\n", host.c_str(), sshUser.c_str());
  bool isOpen = sshClient.begin(host.c_str(), sshUser.c_str(), sshPass.c_str());
  if (!isOpen) debugPrintf("  > SSH authentication failed\n");
  
  // Loop task
  while (true) {
    bool cancel = false;
    // Check ssh channel
    if (!sshClient.available()) {
      debugPrintf("ssh channel lost\n");
      break;
    }

    // host -> minitel
    int nbytes = sshClient.receive();
    if (nbytes < 0) {
      debugPrintf("  > SSH Error while receiving\n");
      break;
    }
    if (nbytes > 0) {
      debugPrintf("[SSH] got %u bytes\n", nbytes);
      int index = 0;
      while (index < nbytes) {
        char b = sshClient.readIndex(index++);
        if (b <= DEL) minitel.writeByte(b); // print only code < 128
        else { // replace char with one "?"
          minitel.writeByte('?');
          // increment index considering utf-8 encoding
          if (b < 0b11100000) index+=1;
          else if (b < 0b11110000) index+=2;
          else index+=3;
        }
      }
    }

    // minitel -> host
    uint32_t key = minitel.getKeyCode(false);
    if (key == 0) {
      vTaskDelay(50/portTICK_PERIOD_MS);
      continue;
    } else if (key == 18) { // CTRL+R = RESET
      break;
    }
    debugPrintf("[KB] got code: 0x%X\n", key);
    switch (key) {
      // redirect minitel special keys
      case SOMMAIRE:   key = 0x07;   break; //BEL : ring
      case GUIDE:      key = 0x07;   break; //BEL : ring
      case ANNULATION: key = 0x0515; break; //ctrl+E ctrl+U : end of line + remove left
      case CORRECTION: key = 0x7F;   break; //DEL : delete
      case RETOUR:     key = 0x01;   break; //ctrl+A : beginning of line
      case SUITE:      key = 0x05;   break; //ctrl+E : end of line
      case REPETITION: key = 0x0C;   break; //ctrl+L : clear-screen (current command repeated)
      case ENVOI:      key = 0x0D;   break; //CR : validate command
      // intercept ctrl+c
      case 0x03: cancel = true; break; 
    }
    // prepare data to send over ssh
    uint8_t payload[4];
    size_t len = 0;
    for (len = 0; key != 0 && len < 4; len++) {
      payload[3-len] = uint8_t(key);
      key = key >> 8;
    }
    if (sshClient.send(payload+4-len, len) < 0) {
      debugPrintf("  > SSH Error while sending\n");
      break;
    }
    // Intercept CTRL+C:
    // displaying data received before host get the command can take minutes
    // we ignore received data to avoid this
    if (cancel) {
      debugPrintf(" > Intercepted ctrl+C\n");
      int nbyte = sshClient.flushReceiving();
      if (col80) {
        minitel.writeByte(0x1b); minitel.println("[0m"); // Reset ANSI/VT100 attributes
      }
      minitel.println();
      minitel.println("\r\r * ctrl+C * ");
      minitel.print("Warning: ");
      minitel.print(String(nbyte));
      minitel.println(" received bytes ignored ");
      minitel.println("as it may takes minutes to display on minitel.");
      // send CR to get new input line
      uint8_t cr = 0x0D;
      sshClient.send(&cr, 1);
    }
  }
  // Closing session
  debugPrintf(" >  Session closed\n");
  sshClient.end();

  // Reinit minitel and Self delete ssh task 
  debugPrintf("\n> SSH task end\n");
  WiFi.disconnect();
  minitel.modeVideotex();
  minitel.moveCursorXY(1, 1);
  minitel.clearScreen();
  minitel.echo(true);
  minitel.pageMode();
  vTaskDelete(NULL);
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
      minitel.moveCursorXY(1, 1);
      minitel.clearScreen();
      minitel.echo(true);
      minitel.pageMode();
      reset();
    }
    debugPrintf("[KB] got code: 0x%X\n", key);
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
/*
      minitel.println();
      minitel.println("DISCONNECTING...");
      delay(3000);
      webSocket.disconnect();
      WiFi.disconnect();
      reset();
*/
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

void writePresets() {
  initFS();
  File file = SPIFFS.open("/telnetpro-presets.cnf", FILE_WRITE);

  for (int i=0; i<20; ++i) {
    DynamicJsonDocument doc(1024);
    doc["presetName"] = presets[i].presetName;
    doc["url"] = presets[i].url;
    doc["scroll"] = presets[i].scroll;
    doc["echo"] = presets[i].echo;
    doc["col80"] = presets[i].col80;
    doc["connectionType"] = presets[i].connectionType;
    doc["ping_ms"] = presets[i].ping_ms;
    doc["protocol"] = presets[i].protocol;
    doc["sshUser"] = presets[i].sshUser;
    doc["sshPass"] = presets[i].sshPass;

    if (serializeJson(doc, file) == 0) {
      debugPrintln(F("Failed to write to file"));
    }
  }
  file.close();
  SPIFFS.end();
}

void readPresets() {
  int countNonEmptySlots = 0;
  initFS();
  File file = SPIFFS.open("/telnetpro-presets.cnf", FILE_READ);
  DynamicJsonDocument doc(1024);
  for (int i=0; i<20; ++i) {
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      presets[i].presetName = "";
      presets[i].url = "";
      presets[i].scroll = false;
      presets[i].echo = false;
      presets[i].col80 = false;
      presets[i].connectionType = 0;
      presets[i].ping_ms = 0;
      presets[i].protocol = "";
      presets[i].sshUser = "";
      presets[i].sshPass = "";
    } else {
      String _presetName = doc["presetName"]; presets[i].presetName = _presetName == "null" ? "" : _presetName;
      String _url = doc["url"]; presets[i].url = _url == "null" ? "" : _url;
      bool _scroll = doc["scroll"]; presets[i].scroll = _scroll;
      bool _echo = doc["echo"]; presets[i].echo = _echo;
      bool _col80 = doc["col80"]; presets[i].col80 = _col80;
      byte _connectionType = doc["connectionType"]; presets[i].connectionType = _connectionType;
      int _ping_ms = doc["ping_ms"]; presets[i].ping_ms = _ping_ms;
      String _protocol = doc["protocol"]; presets[i].protocol = _protocol == "null" ? "" : _protocol;
      String _sshUser  = doc["sshUser"];  presets[i].sshUser  = _sshUser  == "null" ? "" : _sshUser;
      String _sshPass  = doc["sshPass"];  presets[i].sshPass  = _sshPass  == "null" ? "" : _sshPass;

      ++countNonEmptySlots;
    }
  }
  
  file.close();
  SPIFFS.end();

  if (countNonEmptySlots == 0) { // DEFAULT PRESETS IF THERE IS NO PRESET
    presets[0].presetName = "Retrocampus BBS";
    presets[0].url = "bbs.retrocampus.com:1651";
    presets[0].scroll = true;
    presets[0].echo = false;
    presets[0].col80 = false;
    presets[0].connectionType = 0; // Telnet
    presets[0].ping_ms = 0;
    presets[0].protocol = "";
    presets[0].sshUser = "";
    presets[0].sshPass = "";

    presets[1].presetName = "3614 HACKER";
    presets[1].url = "ws:mntl.joher.com:2018/?echo";
    presets[1].scroll = false;
    presets[1].echo = false;
    presets[1].col80 = false;
    presets[1].connectionType = 1; // Websocket
    presets[1].ping_ms = 0;
    presets[1].protocol = "";
    presets[1].sshUser = "";
    presets[1].sshPass = "";

    presets[2].presetName = "3614 TEASER";
    presets[2].url = "ws:minitel.3614teaser.fr:8080/ws";
    presets[2].scroll = false;
    presets[2].echo = false;
    presets[2].col80 = false;
    presets[2].connectionType = 1; // Websocket
    presets[2].ping_ms = 10000;
    presets[2].protocol = "tty";
    presets[2].sshUser = "";
    presets[2].sshPass = "";

    presets[3].presetName = "3615 SM";
    presets[3].url = "wss:wss.3615.live:9991/?echo";
    presets[3].scroll = false;
    presets[3].echo = false;
    presets[3].col80 = false;
    presets[3].connectionType = 1; // Websocket
    presets[3].ping_ms = 0;
    presets[3].protocol = "";
    presets[3].sshUser = "";
    presets[3].sshPass = "";

    presets[4].presetName = "3611.re";
    presets[4].url = "ws:3611.re/ws";
    presets[4].scroll = false;
    presets[4].echo = false;
    presets[4].col80 = false;
    presets[4].connectionType = 1; // Websocket
    presets[4].ping_ms = 0;
    presets[4].protocol = "";
    presets[4].sshUser = "";
    presets[4].sshPass = "";

    presets[5].presetName = "3615co.de";
    presets[5].url = "ws:3615co.de/ws";
    presets[5].scroll = false;
    presets[5].echo = false;
    presets[5].col80 = false;
    presets[5].connectionType = 1; // Websocket
    presets[5].ping_ms = 0;
    presets[5].protocol = "";
    presets[5].sshUser = "";
    presets[5].sshPass = "";

    presets[6].presetName = "TELSTAR by GlassTTY";
    presets[6].url = "glasstty:6502";
    presets[6].scroll = false;
    presets[6].echo = false;
    presets[6].col80 = false;
    presets[6].connectionType = 0; // Telnet
    presets[6].ping_ms = 0;
    presets[6].protocol = "";
    presets[6].sshUser = "";
    presets[6].sshPass = "";

    presets[7].presetName = "ssh example";
    presets[7].url = "[host name or ip]";
    presets[7].scroll = true;
    presets[7].echo = false;
    presets[7].col80 = true;
    presets[7].connectionType = 2; // SSH
    presets[7].ping_ms = 0;
    presets[7].protocol = "";
    presets[7].sshUser = "pi";
    presets[7].sshPass = "raspberry";

    writePresets();
  }
  
}


void reset() {
  ESP.restart();
}
