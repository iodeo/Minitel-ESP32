/*
 * Sample code for connexion to a distant shell through ssh
 * Requirements: ESP32 connected to minitel DIN port and a WiFi connexion
 * 
 * created by iodeo - feb 2023
 * sshClient lib heavily based on jbellue
 */

#include <WiFi.h>
#include <Minitel1B_Hard.h>   // src: https://github.com/eserandour/Minitel1B_Hard.git
#include "sshClient.h"

// ---------------------------------------
// ------ Minitel port configuration

#define MINITEL_PORT Serial2       // for Minitel-ESP32 devboard
#define MINITEL_BAUD 4800          // 1200 / 4800 / 9600 depending on minitel type
#define MINITEL_DISABLE_ECHO true  // set true if characters are repeated when typing
#define MINITEL_COL80 true         // true is recommanded for shell usage, minitel is put in "mode mixte"

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

const char* ssid     = "yourSsid";        // your wifi network
const char* password = "yourPassword";    // your wifi password

// ---------------------------------------
// ------ SSH session config.

const char* ssh_host     = "yourHostIP";   // host ip adress (e.g. "192.168.0.1")
const char* ssh_username = "yourUsername"; // user name (e.g. "pi")
const char* ssh_password = "yourPassword"; // user password (e.g. "raspberrypi")

Minitel minitel(MINITEL_PORT);
SSHClient sshClient;

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
  if (MINITEL_COL80) {
    minitel.modeMixte();
    debugPrintf("  - Mode mixte enabled (80 cols)\n");
  }
  minitel.clearScreen();

  // We connect to WiFi network
  debugPrintf("\n> Wifi setup\n");
  debugPrintf("  Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debugPrintf(".");
  }
  debugPrintf("\n  > WiFi connected with IP %s\n", WiFi.localIP().toString().c_str());

  // We launch SSH session in a new task (stack size needs to be larger)
  debugPrintf("\n> SSH task setup\n");
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(sshTask, "sshTask", 51200, NULL,
    (configMAX_PRIORITIES - 1), NULL, ARDUINO_RUNNING_CORE);
  if (xReturned!=pdPASS) debugPrintf("  > Failed to create task\n");
}

void sshTask(void *pvParameters) {
  debugPrintf("\n> SSH task running\n");
  
  // Open ssh session
  debugPrintf("  Connecting to %s as %s\n", ssh_host, ssh_username);
  bool isOpen = sshClient.begin(ssh_host, ssh_username, ssh_password);
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
        minitel.writeByte(b);
      }
    }

    // minitel -> host
    uint32_t key = minitel.getKeyCode(false);
    if (key == 0) {
      vTaskDelay(50/portTICK_PERIOD_MS);
      continue;
    }
    debugPrintf("[KB] got code: %X\n", key);
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
      debugPrintf(" > Intercepted ctrl+C\n", nbytes);
      int nbyte = sshClient.flush();
      minitel.println();minitel.println();
      minitel.println("\r\r * ctrl+C * ");
      minitel.println("Warning: received data ignored because of it is too long to display");
      minitel.print("number of bytes ignored : ");
      minitel.println(String(nbyte));
      // send CR to get new input line
      uint8_t cr = 0x0D;
      sshClient.send(&cr, 1);
    }
  }
  // Closing session
  debugPrintf(" >  Session closed\n");
  sshClient.end();

  // Self delete task
  vTaskDelete(NULL);
  debugPrintf("\n> SSH task end\n");
}

void loop() {
  // Nothing to do here since sshTaskCode has taken over.
  vTaskDelay(60000 / portTICK_PERIOD_MS);
}
