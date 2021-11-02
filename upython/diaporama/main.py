#!/usr/bin/env python3

from os import listdir
from machine import UART
import upynitel

m = None

def init():
    "Initialisation du serveur vidéotex"
    global m
    m = upynitel.Pynitel(UART(2, baudrate = 1200,parity=0, bits=7,stop=1))
    
def accueil():
    m.home()
    m.pos(2)
    m._print("Use case prototype")
    m.pos(3)
    m.plot('-', 40)
    m.pos(6,5)
    m.scale(3)
    m._print("FLASH DIAPORAMA")
    m.pos(10)
    m.scale(0)
    m._print(" * Affichage d'écrans videotex")
    m.pos(11)
    m._print("   stocké dans la memoire flash")
    m.pos(13)
    m._print(" * Navigation avec les touches")
    m.pos(14)
    m._print("   RETOUR et SUITE   ")
    m.pos(16)
    m._print(" * Appuyer ENVOI pour commencer")
    m.pos(1)
    
    while True:
        (choix, touche) = m.input(0, 1, 0, '')
        if touche == m.envoi:
            break
        
def diapo():
    global m
    
    fileList = listdir('/ecrans')
    numFiles = len(fileList)
    num = 0;
    
    while True:
        m.home()
        m.xdraw('/ecrans/' + fileList[num])
        (choix, touche) = m.input(0, 1, 0, '')
        m.cursor(False)
        if touche == m.suite:
            num = num+1
        elif touche == m.retour:
            num = num-1

init()
accueil()
diapo()
