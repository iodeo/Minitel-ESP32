
This software is a quick adaptation of [pynitel library] (https://github.com/cquest/pynitel), made by [cquest under AGPL license.

upynitel functions help to handle Minitel screen and keyboard exactly as pynitel does.

main.py is based on annuaire_exemple.py with fake annuaire request in order to preserve ESP32 which is not able to load a full webpage in dynamic memory. 
Keeping only usefull parts of incoming data, would solve the problem.


## Quick guidance to micropython on ESP32: 
1. Put micropython firmware into your ESP32
2. Connect it to a Python IDE with micropython support (Thonny IDE is a good way to go)
3. Put the .py files into ESP32 using your IDE
   * boot.py, equiv. to Arduino setup()
   * main.py, equiv. to Arduino loop()
   * upynitel.py, as a local library

Hints (Thonny IDE):
* CTRL+C Stop running
* CTRL+D Soft reboot


This is just to give an idea for people new to micropython.

For complete guidance and tutorials, Please check on the internet ;)
