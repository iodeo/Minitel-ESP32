////////////////////////////////////////////////////////////////////////
/*
   Minitel1B_Hard - Démo - Version du 11 juin 2017 à 16h00
   Copyright 2016, 2017 - Eric Sérandour
   
   >> Légèrement adapté pour l'ESP32 par iodeo
   
   Documentation utilisée :
   Spécifications Techniques d'Utilisation du Minitel 1B
   http://543210.free.fr/TV/stum1b.pdf
   
////////////////////////////////////////////////////////////////////////
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

// DEBUT DU PROGRAMME

////////////////////////////////////////////////////////////////////////

#include <Minitel1B_Hard.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>

#define FORMAT_SPIFFS_IF_FAILED true



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

Minitel minitel(Serial2);  // Le port utilisé sur ESP32


int wait = 10000;

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);  // Le port de débug
  minitel.changeSpeed(minitel.searchSpeed());
  //minitel.changeSpeed(1200);
  minitel.echo(false);
  minitel.extendedKeyboard(); // activation des touches curseurs

}


void initFS() {
  boolean ok = SPIFFS.begin();
  if (!ok)
  {
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

////////////////////////////////////////////////////////////////////////

void loop() {
  mainFunction();
  do {
    delay(wait); 
  } while (1);
  demoCaracteres();
  demoGraphic();
  demoTailles();
  demoCouleurs();
  demoCurseur();
}



typedef struct {
  String presetName = "";
  
  String url = "";
  bool scroll = false;
  bool echo = false;
  bool col80 = false;
  byte connectionType = 0;
  int ping_ms = 0;
  String protocol = "";
} Preset;

Preset presets[20];



void mainFunction() {
  String baudStr = String(minitel.searchSpeed());
  minitel.newScreen();
  minitel.textMode();
  


/*
  initFS();
  File file = SPIFFS.open("/test.txt", FILE_WRITE);

  for (int i=0; i<3; ++i) {
    presets[i].presetName = "Preset n." + String(i+1);
    presets[i].url = "ws://hostnumber" + String(i+1) + ".com/?echo";
    presets[i].connectionType = i;
    presets[i].ping_ms = i*100;
  }

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

    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }

  
  }

  file.close();
  SPIFFS.end();
*/








/*
  initFS();
listDir(SPIFFS, "/", 1);
readFile(SPIFFS, "/test.txt");
  SPIFFS.end();
minitel.println("Fatto 2.");
*/







initFS();
File file = SPIFFS.open("/test.txt", FILE_READ);
if (file) minitel.println("SIII"); else minitel.println("NOOOOO");
DynamicJsonDocument doc(1024);
for (int i=0; i<8; ++i) {
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
  } else {
    String _presetName = doc["presetName"]; presets[i].presetName = _presetName == "null" ? "" : _presetName;
    String _url = doc["url"]; presets[i].url = _url == "null" ? "" : _url;
    bool _scroll = doc["scroll"]; presets[i].scroll = _scroll;
    bool _echo = doc["echo"]; presets[i].echo = _echo;
    bool _col80 = doc["col80"]; presets[i].col80 = _col80;
    byte _connectionType = doc["connectionType"]; presets[i].connectionType = _connectionType;
    int _ping_ms = doc["ping_ms"]; presets[i].ping_ms = _ping_ms;
    String _protocol = doc["protocol"]; presets[i].protocol = _protocol == "null" ? "" : _protocol;
  }
}

file.close();
SPIFFS.end();

  






  while (true) {
    delay(1000);
  }
  minitel.moveCursorXY(12,1);
  minitel.attributs(DOUBLE_HAUTEUR);
  minitel.print("Comune di Bugliano");
  minitel.moveCursorXY(12,4);
  for (int i=1; i<=18; i++) minitel.writeByte(0x7E);
  minitel.attributs(GRANDEUR_NORMALE);
  minitel.moveCursorXY(12,3);
  minitel.attributs(CARACTERE_VERT);
  minitel.print("Servizi Telematici");
  minitel.attributs(CARACTERE_BLANC);
  minitel.moveCursorXY(2, 6); minitel.attributs(INVERSION_FOND); minitel.print(" 1 "); minitel.attributs(FOND_NORMAL); minitel.print(" Tassa di soggiorno");
  minitel.moveCursorXY(2, 8); minitel.attributs(INVERSION_FOND); minitel.print(" 2 "); minitel.attributs(FOND_NORMAL); minitel.print(" Pro loco");
  minitel.moveCursorXY(2,10); minitel.attributs(INVERSION_FOND); minitel.print(" 3 "); minitel.attributs(FOND_NORMAL); minitel.print(" Sagra della Buglianella");
  minitel.moveCursorXY(2,12); minitel.attributs(INVERSION_FOND); minitel.print(" 4 "); minitel.attributs(FOND_NORMAL); minitel.print(" Servizi BugliaCom");
  minitel.moveCursorXY(2,14); minitel.attributs(INVERSION_FOND); minitel.print(" 5 "); minitel.attributs(FOND_NORMAL); minitel.print(" Tasse e tributi");
  minitel.moveCursorXY(2,16); minitel.attributs(INVERSION_FOND); minitel.print(" 6 "); minitel.attributs(FOND_NORMAL); minitel.print(" Palio del lancio del gatto");
  minitel.moveCursorXY(2,18);
  minitel.println();

  
/*
  String s = "AB";
  s.concat("c");
  s.concat('d');
  s.remove(s.length()-1);
  minitel.print(s);
 */

minitel.print("> ");
String name = inputString();
Serial.printf("Out=%s\n",name);


SPIFFS.begin();
listDir(SPIFFS, "/", 1);
readFile(SPIFFS, "/test.txt");

SPIFFS.end();
/*
  int i=0;
  do {
    unsigned long key = minitel.getKeyCode();
    if (key != 0) Serial.printf("key = %u   %d\n",key,i++);
    delay(1000);
    // INVIO=4929
    // CORREZIONE=4935
    // INVIOVERO=13
    // INDICE=4934
    // ANNUL=4933
    // PRECEDENTE=4930
    // SUCCESSIVO=4936
    // RIPETIZ=4931
    // GUIDA=4932
    // FRECCIA SU=1792833
    // FRECCIA GIU=1792834
    // FRECCIA SX=1792836
    // FRECCIA DX=1792835
    // CTRL+C=3
    // ESC=27
  } while (true);
*/
 
}

String inputString() {
  minitel.cursor();
  String out = "";
  unsigned long key = minitel.getKeyCode();
  while (!(
      key == 4929 || // Invio
      key == 13   ||
      key == 10   ||
      key == 27   || // ESC
      key == 3       // CTRL+C
    )) {
    if (key != 0) {
      Serial.printf("Key = %u\n",key);
      if (key >= 32 && key <= 127) {
        out.concat((char)key);
        minitel.printChar(key);
      } else if (out.length() > 0 && (key == 8 || key == 4935)) {
        out.remove(out.length()-1);
        minitel.noCursor();
        minitel.moveCursorLeft(1);
        minitel.print(" ");
        minitel.moveCursorLeft(1);
        minitel.cursor();
      }
    }
    key = minitel.getKeyCode();
 }
 minitel.noCursor();
 minitel.println();
 return out;
}

////////////////////////////////////////////////////////////////////////

void newPage(String titre) {
  minitel.newScreen();
  minitel.println(titre);
  for (int i=1; i<=40; i++) {
    minitel.writeByte(0x7E);
  }
  minitel.moveCursorReturn(1); 
}

////////////////////////////////////////////////////////////////////////

void demoCaracteres() {
  newPage("LES CARACTERES");
 // minitel.println("Inizio test");
    //String baudStr = String(minitel.searchSpeed());
    //minitel.println(baudStr);
/*
  do {
    //minitel.println(baudStr);
  } while (1);
*/
  // Mode texte 
  
  minitel.println("MODE TEXTE SANS LIGNAGE :");
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.moveCursorReturn(2);
  
  minitel.println("MODE TEXTE AVEC LIGNAGE :");
  minitel.attributs(DEBUT_LIGNAGE);  // En mode texte, le lignage est déclenché par le premier espace rencontré (0x20).
  for (int i=0x20; i<=0x7F; i++) {   
    minitel.writeByte(i);   
  }
  minitel.attributs(FIN_LIGNAGE);
  minitel.moveCursorReturn(2);  

  // Mode semi-graphique
  
  minitel.textMode();
  minitel.println("MODE SEMI-GRAPHIQUE SANS LIGNAGE :");
  minitel.graphicMode();  
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.moveCursorReturn(2);
  
  minitel.textMode();
  minitel.println("MODE SEMI-GRAPHIQUE AVEC LIGNAGE :");
  minitel.graphicMode();
  minitel.attributs(DEBUT_LIGNAGE);
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.attributs(FIN_LIGNAGE);
  minitel.moveCursorReturn(2);
    
  delay(wait); 
}

////////////////////////////////////////////////////////////////////////

void demoGraphic() {
  newPage("LA FONCTION GRAPHIC");
  minitel.textMode();
  minitel.println("Un caractère semi-graphique est composé de 6 pseudo-pixels :");
  minitel.println();
  minitel.graphicMode();
  minitel.attributs(DEBUT_LIGNAGE);
  minitel.writeByte(0x7F);
  minitel.attributs(FIN_LIGNAGE);
  minitel.textMode();
  minitel.print(" avec lignage ou ");
  minitel.graphicMode();
  minitel.writeByte(0x7F);
  minitel.textMode();
  minitel.println(" sans lignage.");
  minitel.println();
  String chaine = "";
  chaine += "minitel.graphic(0b101011) donne ";
  minitel.textMode();
  minitel.print(chaine);
  minitel.graphicMode();
  minitel.graphic(0b101011);
  minitel.textMode();
  minitel.println();
  minitel.println();
  chaine = "";
  chaine += "minitel.graphic(0b110110,30,15) donne ";
  minitel.print(chaine);  
  minitel.graphicMode();
  minitel.graphic(0b110110,30,15);
  minitel.noCursor();
  delay(2*wait); 
}

////////////////////////////////////////////////////////////////////////

void demoTailles() { 
  newPage("LES TAILLES");
  minitel.println("GRANDEUR_NORMALE");
  minitel.attributs(DOUBLE_HAUTEUR);
  minitel.print("DOUBLE_HAUTEUR");  
  minitel.attributs(DOUBLE_LARGEUR);
  minitel.println();
  minitel.println("DOUBLE_LARGEUR");  
  minitel.attributs(DOUBLE_GRANDEUR); 
  minitel.println("DOUBLE_GRANDEUR");
  minitel.println();
  minitel.attributs(GRANDEUR_NORMALE);
  minitel.attributs(DEBUT_LIGNAGE);  // En mode texte, le lignage est déclenché par le premier espace rencontré (0x20).
  minitel.println("SEULEMENT EN MODE TEXTE");  
  minitel.attributs(FIN_LIGNAGE);
  minitel.println();
  delay(wait);  
}

////////////////////////////////////////////////////////////////////////

void demoCouleurs() {
  newPage("LES COULEURS");
  for (int i=0; i<=1; i++) {   
    if (i==0) { minitel.textMode(); }
    if (i==1) { minitel.graphicMode(); }
    minitel.attributs(INVERSION_FOND);
    minitel.print("CARACTERE_NOIR, FOND_BLANC");
    minitel.attributs(FOND_NORMAL);
    minitel.println(" (INVERSION)");      
    minitel.attributs(CARACTERE_ROUGE);
    minitel.println("CARACTERE_ROUGE");
    minitel.attributs(CARACTERE_VERT);
    minitel.println("CARACTERE_VERT");
    minitel.attributs(CARACTERE_JAUNE);
    minitel.println("CARACTERE_JAUNE");
    minitel.attributs(CARACTERE_BLEU);
    minitel.println("CARACTERE_BLEU");
    minitel.attributs(CARACTERE_MAGENTA);
    minitel.println("CARACTERE_MAGENTA");
    minitel.attributs(CARACTERE_CYAN);
    minitel.println("CARACTERE_CYAN");
    minitel.attributs(CARACTERE_BLANC);
    minitel.println("CARACTERE_BLANC");
    minitel.println();
  }
  delay(wait);
}

////////////////////////////////////////////////////////////////////////

void demoCurseur() {
  minitel.cursor();
  newPage("DEPLACER LE CURSEUR");
  minitel.moveCursorXY(20,12);
  for (int i=1; i<=100; i++) {
    delay(100);
    switch (random(4)) {
      case 0: minitel.moveCursorRight(1+random(3)); break;
      case 1: minitel.moveCursorLeft(1+random(3)); break;
      case 2: minitel.moveCursorDown(1+random(3)); break;
      case 3: minitel.moveCursorUp(1+random(3)); break;
    }
  }
  newPage("POSITIONNER LE CURSEUR");
  minitel.textMode();
  for (int i=1; i<1000; i++) {
    minitel.moveCursorXY(1+random(40),3+random(22));
    minitel.writeByte(0x20 + random(0x60));
  }
  
  minitel.newScreen();
  minitel.textMode();
  minitel.noCursor();
  for (int i=1; i<1000; i++) {
    if (random(4)<3) { minitel.textMode(); }
    else {
      minitel.graphicMode();
      minitel.attributs(DEBUT_LIGNAGE);
    }
    minitel.attributs(0x4C+random(5));
    minitel.writeByte(0x20 + random(0x60));
    minitel.attributs(FIN_LIGNAGE);
  }
}

///////////////////////////////////////////////////////////////////////


















void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
   Serial.printf("Listing directory: %s\r\n", dirname);

   File root = fs.open(dirname);
   if(!root){
      Serial.println("− failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println(" − not a directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
            listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("\tSIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}

void readFile(fs::FS &fs, const char * path){
   Serial.printf("Reading file: %s\r\n", path);

   File file = fs.open(path);
   if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return;
   }

   Serial.println("− read from file:");
   while(file.available()){
      Serial.write(file.read());
   }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\r\n", path);

   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("− failed to open file for writing");
      return;
   }
   if(file.print(message)){
      Serial.println("− file written");
   }else {
      Serial.println("− frite failed");
   }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Appending to file: %s\r\n", path);

   File file = fs.open(path, FILE_APPEND);
   if(!file){
      Serial.println("− failed to open file for appending");
      return;
   }
   if(file.print(message)){
      Serial.println("− message appended");
   } else {
      Serial.println("− append failed");
   }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
   Serial.printf("Renaming file %s to %s\r\n", path1, path2);
   if (fs.rename(path1, path2)) {
      Serial.println("− file renamed");
   } else {
      Serial.println("− rename failed");
   }
}

void deleteFile(fs::FS &fs, const char * path){
   Serial.printf("Deleting file: %s\r\n", path);
   if(fs.remove(path)){
      Serial.println("− file deleted");
   } else {
      Serial.println("− delete failed");
   }
}










void separateUrl(String url, String &host, uint32_t &port, bool &ssl, String &path) {

  url.trim();
  String temp = String(url);
  temp.toLowerCase();

  if (temp.startsWith("wss://")) {
    ssl = true;
    url.remove(0, 6);
  } else if (temp.startsWith("ws://")) {
    ssl = false;
    url.remove(0, 5);
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
}
