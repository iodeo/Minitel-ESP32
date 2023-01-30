#include <Minitel1B_Hard.h>
#include <Preferences.h>
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
Preferences prefs;

// WiFi credentials
String ssid("");
String password("");

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
        char *reset = NULL;
        *reset = 1;
      }
    }
    debugPrintln();
    debugPrint("WiFi connected with local IP: ");
    debugPrintln(WiFi.localIP());
    minitel.print("Connected with IP ");
    minitel.println(WiFi.localIP().toString());
  
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
      char *reset = NULL;
      *reset = 1;
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
      } else if (out.length() > 0 && (key == 8 || key == 4935)) {
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
        char *reset = NULL;
        *reset = 1;
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
  host = prefs.getString("host", "");
  port = prefs.getUInt("port", 0);
  scroll = prefs.getBool("scroll", false);
  echo = prefs.getBool("echo", false);
  col80 = prefs.getBool("col80", false);
  prefs.end();
}

void savePrefs() {
  prefs.begin("telnet-pro", false);
  if (prefs.getString("ssid",     "") != ssid)     prefs.putString("ssid", ssid);
  if (prefs.getString("password", "") != password) prefs.putString("password", password);
  if (prefs.getString("host",     "") != host)     prefs.putString("host", host);
  if (prefs.getUInt("port", 0) != port) prefs.putUInt("port",   port);
  if (prefs.getBool("scroll", false) != scroll) prefs.putBool("scroll", scroll);
  if (prefs.getBool("echo",   false) != echo)   prefs.putBool("echo",   echo);
  if (prefs.getBool("col80",  false) != col80)  prefs.putBool("col80",  col80);
  prefs.end();
}

void showPrefs() {
  minitel.noCursor();
  minitel.moveCursorXY(9,1);
  minitel.attributs(FIN_LIGNAGE);
  minitel.attributs(DOUBLE_HAUTEUR); minitel.attributs(CARACTERE_JAUNE); minitel.attributs(INVERSION_FOND); minitel.println("  Minitel Telnet Pro  ");
  minitel.attributs(FOND_NORMAL); minitel.attributs(GRANDEUR_NORMALE);
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("1"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("SSID: "); minitel.attributs(CARACTERE_CYAN); printStringValue(ssid); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("2"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Pass: "); minitel.attributs(CARACTERE_CYAN); printPassword(); minitel.clearLineFromCursor(); minitel.println();
  minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("3"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Host: "); minitel.attributs(CARACTERE_CYAN); printStringValue(host); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("4"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Port: "); minitel.attributs(CARACTERE_CYAN); minitel.print(String(port)); minitel.clearLineFromCursor(); minitel.println();
  minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("5"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Scroll: "); writeBool(scroll); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("6"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Echo  : "); writeBool(echo); minitel.clearLineFromCursor(); minitel.println();
  minitel.attributs(CARACTERE_BLANC); minitel.graphicMode(); minitel.writeByte(0x6A); minitel.textMode(); minitel.attributs(INVERSION_FOND); minitel.print("7"); minitel.attributs(FOND_NORMAL); minitel.graphicMode(); minitel.writeByte(0x35); minitel.textMode(); minitel.print("Col80 : "); writeBool(col80); minitel.clearLineFromCursor(); minitel.println();
  
  minitel.moveCursorXY(16,15);
  minitel.attributs(DOUBLE_LARGEUR); minitel.attributs(CARACTERE_MAGENTA); // minitel.attributs(CLIGNOTEMENT); 
  minitel.print("PRESS");
  minitel.moveCursorXY(16,16);
  minitel.attributs(DOUBLE_GRANDEUR);
  minitel.print("SPACE");
  minitel.moveCursorXY(10,18);
  minitel.attributs(DOUBLE_LARGEUR);
  minitel.print("TO CONNECT");
  minitel.attributs(GRANDEUR_NORMALE); minitel.attributs(CARACTERE_BLANC); //minitel.attributs(FIXE);
}

void printPassword() {
  if (password == NULL || password == "") {
    minitel.print("-undefined-");
  } else {
    for (int i=0; i<password.length(); ++i) minitel.print("*");
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
  while (key != 32) {
    if (key != 0) {
      Serial.printf("Key = %u\n", key);
      if (key == 18) { // CTRL+R = RESET
        minitel.modeVideotex();
        minitel.clearScreen();
        minitel.moveCursorXY(1, 1);
        minitel.echo(true);
        minitel.pageMode();
        char *reset = NULL;
        *reset = 1;
      } else if (key == '1') {
        setParameter(10, 4, ssid, false);
      } else if (key == '2') {
        setParameter(10, 5, password, true);
      } else if (key == '3') {
        setParameter(10, 7, host, false);
      } else if (key == '4') {
        setIntParameter(10, 8, port);
      } else if (key == '5') {
        switchParameter(12, 10, scroll);
      } else if (key == '6') {
        switchParameter(12, 11, echo);
      } else if (key == '7') {
        switchParameter(12, 12, col80);
      }
    }
    if (key >= '1' && key <= '7') {
      minitel.moveCursorXY(1,24); minitel.attributs(CARACTERE_BLANC); minitel.print("WAIT");
      savePrefs();
      minitel.moveCursorXY(1,24); minitel.clearLineFromCursor();
    }
    key = minitel.getKeyCode();
  }
  minitel.clearScreen();
  minitel.moveCursorXY(1, 1);
}

void switchParameter(int x, int y, bool &destination) {
  destination = !destination;
  minitel.moveCursorXY(x,y); writeBool(destination);
}

void setParameter(int x, int y, String &destination, bool mask) {
  minitel.moveCursorXY(x,y); minitel.attributs(CARACTERE_BLANC);
  minitel.print(destination);
  for (int i=0; i<31-destination.length(); ++i) minitel.print(".");
  minitel.moveCursorXY(x,y);
  int exitCode = 0;
  String temp = inputString(destination, exitCode, '.');
  if (!exitCode && temp.length() > 0) {
    destination = String(temp);
  }
  minitel.moveCursorXY(x,y); minitel.attributs(CARACTERE_CYAN);
  if (destination == "") {
    minitel.print("-undefined-");
  } else {
    if (mask)
      for (int i=0; i<destination.length(); ++i) minitel.print("*");
    else
      printStringValue(destination);
  }
  minitel.clearLineFromCursor();
}

void setIntParameter(int x, int y, uint16_t &destination) {
  String strParam = String(destination);
  if (strParam == "0") strParam = "";
  minitel.moveCursorXY(x,y); minitel.attributs(CARACTERE_BLANC);
  minitel.print(strParam);
  for (int i=0; i<31-String(destination).length(); ++i) minitel.print(".");
  minitel.moveCursorXY(x,y);
  int exitCode = 0;
  String temp = inputString(strParam, exitCode, '.');
  if (!exitCode && temp.length() > 0) {
    destination = temp.toInt();
  }
  minitel.moveCursorXY(x,y); minitel.attributs(CARACTERE_CYAN);
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
