## How to

Some instructions are given in [hackaday project page](https://hackaday.io/project/180473/instructions)

## Arduino IDE Setttings

If the Arduino IDE doesn't have by default the board "ESP32 Dev Module", then go to **Settings** (Arduino IDE menu), then be sure that in **Additional boards manager URLs** there is:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

## Libraries

Where name is starting by "Minitel1B", the following lib must be added:

https://github.com/eserandour/Minitel1B_Hard

**Minitel1B_Websocket_Client** and **Minitel1B_Telnet_Pro** require this additional lib:

https://github.com/Links2004/arduinoWebSockets

**Minitel1B_SSH_Client** and **Minitel1B_Telnet_Pro** require this additional lib:

https://github.com/ewpa/LibSSH-ESP32

**Minitel1B_Telnet_Pro** require this additional lib:

https://github.com/bblanchon/ArduinoJson
