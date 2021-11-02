## What is this ?

Examples for upynitel library 

upynitel is a quick adaptation of [pynitel library](https://github.com/cquest/pynitel), made by cquest under AGPL license.

upynitel functions help to handle Minitel screen and keyboard exactly as pynitel does.

## Examples description
* upynitel/main.py is based on annuaire_exemple.py with fake annuaire request in order to preserve ESP32 memory which is not able to load a full webpage from http request. Keeping only usefull parts of incoming data, would solve the problem.
* diaporama/main.py is a diaporama application displaying vdt screens from ESP32 flash memory. Screens are from (XReyRobert)[https://github.com/XReyRobert/VideotexPagesRepository/]

## Quick guidance to micropython on ESP32: 
1. Put micropython firmware into your ESP32
2. Connect it to a Python IDE with micropython support (Thonny IDE is a good way to go)
3. Put the .py files into ESP32 using your IDE
   * boot.py
   * main.py
   * upynitel.py

Hints (Thonny IDE):
* CTRL+C Stop running
* CTRL+D Soft reboot


This is just to give an idea for people new to micropython.

For complete guidance and tutorials, Please check on the internet ;)
