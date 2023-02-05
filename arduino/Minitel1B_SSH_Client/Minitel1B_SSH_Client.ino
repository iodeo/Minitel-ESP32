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

const char* ssid     = "SFR_8B60";        // your wifi network
const char* password = "henriouf-dionnet@coucy.MQ";    // your wifi password

// ---------------------------------------
// ------ SSH session config.

const char* ssh_host     = "192.168.1.51"; // host ip adress (e.g. "192.168.0.1")
const char* ssh_username = "kiwi";         // user name (e.g. "pi")
const char* ssh_password = "kiwi.netAC3";  // user password (e.g. "raspberrypi")

//WiFiClient client
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

  // We connect to host and open ssh session
  debugPrintf("\n> SSH session setup\n");
  debugPrintf("  Connecting to %s as %s\n", ssh_host, ssh_username);
  if (sshClient.begin(ssh_host, ssh_username, ssh_password) != SSHClient::SSHStatus::OK) {
    debugPrintf("  > SSH authentication failed\n");
    while(1) delay(500); // endless
  }
  else debugPrintf("  > SSH authentication ok\n");
  
  debugPrintf("\n> End of setup\n\n");

}

void loop() {

  // Check ssh channel
  if (!sshClient.available()) {
    debugPrintf("ssh channel lost\n");
    delay(1000);
    return;
  }

  // host -> minitel
  int nbytes = sshClient.receive();
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
  if (key != 0) {
    debugPrintf("[KB] got code: %X\n", key);
    // prepare data to send over ssh
    uint8_t payload[4];
    size_t len = 0;
    for (len = 0; key != 0 && len < 4; len++) {
      payload[3-len] = uint8_t(key);
      key = key >> 8;
    }
    sshClient.send(payload+4-len, len);
  }
  
}
