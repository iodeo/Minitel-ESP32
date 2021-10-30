## BREADBOARD VERSION
This schematic is an example of what can be made with cheap ESP32 modules.
<br>WARNING : This schematic may not be powered with both minitel and usb power sources at the same time.
<br>The DIN connector may be replaced by home made connector from any DIN5 cable.
<br>Buck converter is only needed where USB power source is not available.
<br>Any buck converter with voltage input range from 6v to 15v with power rating greater than 1 Amps would fit.
An output voltage of 5 volts may also be used with the 5v to 3v3 regulator of the ESP32 module; tension divisor on NPN base may be adapted to ensure good transmission.
<br>Serial trnasmission is wired to ESP32 module through RX2 & TX2 pins (usually P16 & P17)

## DEVBOARD VERSION
Devboard version includes USB support, Autoboot selection mode, Integrated buck converter with fuse protection, Power management to enable USB and/or minitel power sources, Serial adapter to minitel, various status LEDs.
Devboard version is available on demand at [contact@iodeo.fr](contact@iodeo.fr)
