## How to

Some instructions are given in [hackaday project page](https://hackaday.io/project/180473/instructions)

## Minitel1B

Where name is starting by "Minitel1B", the following lib must be added:

https://github.com/eserandour/Minitel1B_Hard

**Minitel1B_Websocket_Client** and **Minitel1B_Telnet_Pro** require this additional lib:

https://github.com/Links2004/arduinoWebSockets.git

## libssh

For LibSSH_Shell, the following lib must be added:

https://github.com/ewpa/LibSSH-ESP32

In current version, Minitel needs to be set manually in 4800 bauds (Fnct+P 4) and in teleinformatique standard (Fcnt+T A) before program starts. 
Dev board reset button is usefull for this.

Once connected to server, you will also want to disable local Echo (Fnct+T E) because echo is provided through ssh already.
