#include "Telnet_Client.h"
// -------------------------------------------------------------------

bool TelnetClient::connect(IPAddress ip, uint16_t port) {
  return client.connect(ip, port);
}
// -------------------------------------------------------------------

int TelnetClient::available() {
  return client.available();
}
// -------------------------------------------------------------------

bool TelnetClient::connected() {
  return client.connected();
}
// -------------------------------------------------------------------

char TelnetClient::readChar() {
  return (char) client.read();
}
// -------------------------------------------------------------------

void TelnetClient::println(String chaine) {
  for (int i=0; i<chaine.length(); i++) {
    unsigned char caractere = chaine.charAt(i);
    client.write(caractere);
  }
  client.write('\n');
}

// -------------------------------------------------------------------
