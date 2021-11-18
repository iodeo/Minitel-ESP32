## What is this ?

Examples for upynitel library 

upynitel is a quick adaptation of [pynitel library](https://github.com/cquest/pynitel), made by cquest under AGPL license.

upynitel functions help to handle Minitel screen and keyboard exactly as pynitel does.

## Examples description

All examples are written for communication at 1200 bauds in videotex standard mode.

* upynitel/main.py is a minimal sample code. It only imports required libraries and declare a pynitel instance in order to start playing with your Minitel from thonny IDE shell.
* fake_annuaire/main.py is based on annuaire_exemple.py with fake annuaire request in order to preserve ESP32 memory which is not able to load a full webpage from http request. Keeping only usefull parts of incoming data, would solve the problem.
* diaporama/main.py is a diaporama application displaying vdt screens from ESP32 flash memory. Screens are from [XReyRobert](https://github.com/XReyRobert/VideotexPagesRepository/)

## Quick guidance to micropython on ESP32
1. Put micropython firmware into your ESP32
2. Connect it to a Python IDE with micropython support, such as Thonny IDE
3. Put the .py files into ESP32 using your IDE
   * boot.py
   * main.py
   * upynitel.py

Hints (Thonny IDE):
* CTRL+C Stop running
* CTRL+D Soft reboot

More instructions are given in [hackaday project page](https://hackaday.io/project/180473/instructions)
