## BREADBOARD VERSION
This schematic is an example of what can be made with cheap ESP32 modules.

WARNING : This circuit may not be powered from minitel and usb at the same time.

The DIN connector may be replaced by home made connector from any DIN5 cable.


Buck converter is only needed where USB power source is not available.

Any buck converter with voltage input range from 6v to 15v with power rating greater than 1 Amps would fit.
<br>An output voltage of 5 volts may also be used in conjunction with ESP32 module regulator; tension divisor on NPN base may be adapted to ensure good transmission.

Serial transmission is wired to ESP32 module through RX2 & TX2 pins (usually P16 & P17)

## DEVBOARD VERSION
ESP MINITEL DEVBOARD v2 schematic is provided.

The devboard includes :
* USB support (USB-C port)
* Auto bootmode selection
* Integrated buck converter with fuse protection
* Power management to enable USB and/or minitel plugged at any time
* Serial connection to minitel of course
* And some status LEDs

The pcb has same size as esp32 common modules thanks to two sided assembly. 
<br>It can be fitted with DIN or JST-XH connector.

Available on demand at [contact@iodeo.fr](mailto:contact@iodeo.fr)
