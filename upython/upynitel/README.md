## Upynitel library 

upynitel is a quick adaptation of [pynitel library](https://github.com/cquest/pynitel), made by cquest under AGPL license.

upynitel functions help to handle Minitel screen and keyboard exactly as pynitel does.

## Examples description

All examples are written for communication at 1200 bauds in videotex standard mode.

* upynitel/main.py is a minimal sample code. It only imports required libraries and declare a pynitel instance in order to start playing with your Minitel from thonny IDE shell.
* fake_annuaire/main.py is based on annuaire_exemple.py with fake annuaire request in order to preserve ESP32 memory which is not able to load a full webpage from http request. Keeping only usefull parts of incoming data, would solve the problem.
* diaporama/main.py is a diaporama application displaying vdt screens from ESP32 flash memory. Screens are from [XReyRobert](https://github.com/XReyRobert/VideotexPagesRepository/)
