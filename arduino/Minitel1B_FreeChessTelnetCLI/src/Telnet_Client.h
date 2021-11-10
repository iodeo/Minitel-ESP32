// 
// iodeo, 2021
// based on hector 04/04/18.
// 

#ifndef TELNET_CLIENT_H
#define TELNET_CLIENT_H

#include <WString.h>
#include <WiFi.h>

class TelnetClient {
  
public:

  bool connect(IPAddress ip, uint16_t port);
  int available();
  bool connected();
  char readChar();               // read character from server
  void println(String chaine);   // write string to server

private:

  WiFiClient client;
  
};

#endif //TELNETCLIENT_TELNETCLIENT_H
