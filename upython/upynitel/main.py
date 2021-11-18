#!/usr/bin/env python3

from machine import UART
import upynitel

m = upynitel.Pynitel(UART(2, baudrate = 1200,parity=0, bits=7,stop=1))
