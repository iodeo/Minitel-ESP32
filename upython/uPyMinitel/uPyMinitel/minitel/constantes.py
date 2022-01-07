#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Définitions de constantes de l’univers Minitel"""

# Codes de contrôles de la norme ASCII
NUL = 0x00 # null
SOH = 0x01 # start of heading
STX = 0x02 # start of text
ETX = 0x03 # end of text
EOT = 0x04 # end of transmission
ENQ = 0x05 # enquiry
ACK = 0x06 # acknowledge
BEL = 0x07 # bell
BS  = 0x08 # backspace
TAB = 0x09 # horizontal tab
LF  = 0x0a # line feed, new line
VT  = 0x0b # vertical tab
FF  = 0x0c # form feed, new page
CR  = 0x0d # carriage return
SO  = 0x0e # shift out
SI  = 0x0f # shift in
DLE = 0x10 # data link escape
DC1 = 0x11 # device control 1
CON = 0x11 # Cursor on
DC2 = 0x12 # device control 2
REP = 0x12 # Rep
DC3 = 0x13 # device control 3
SEP = 0x13 # Sep
DC4 = 0x14 # device control 4
COF = 0x14 # Cursor off
NAK = 0x15 # negative acknowledge
SYN = 0x16 # synchronous idle
ETB = 0x17 # end of transmission block
CAN = 0x18 # cancel
EM  = 0x19 # end of medium
SS2 = 0x19 # SS2
SUB = 0x1a # substitute
ESC = 0x1b # escape
FS  = 0x1c # file separator
GS  = 0x1d # group separator
SS3 = 0x1d # SS3
RS  = 0x1e # record separator
US  = 0x1f # unit separator

PRO1 = [ESC, 0x39] # protocole 1
PRO2 = [ESC, 0x3a] # protocole 2
PRO3 = [ESC, 0x3b] # protocole 3
CSI  = [ESC, 0x5b] # CSI

# Commandes PRO1
DECONNEXION = 0x67
CONNEXION = 0x68
RET1 = 0x6c
RET2 = 0x6d
OPPO = 0x6f
STATUS_TERMINAL = 0x70
STATUS_CLAVIER = 0x72
STATUS_FONCTIONNEMENT = 0x72
STATUS_VITESSE = 0x74
STATUS_PROTOCOLE = 0x76
ENQROM = 0x7b
RESET = 0x7f

# Commandes PRO2
COPIE = 0x7c
AIGUILLAGE_TO = 0x62
NON_DIFFUSION = 0x64
NON_RETOUR_ACQUITTEMENT = 0x64
DIFFUSION = 0x65
RETOUR_ACQUITTEMENT = 0x65
TRANSPARENCE = 0x66
START = 0x69
STOP = 0x6a
PROG = 0x6b
REP_STATUS_TERMINAL = 0x71
REP_STATUS_CLAVIER = 0x73
REP_STATUS_FONCTIONNEMENT = 0x73
REP_STATUS_VITESSE = 0x75
REP_STATUS_PROTOCOLE = 0x77
TELINFO = [0x31, 0x7d]
MIXTE1 = [0X32, 0X7d]
MIXTE2 = [0X32, 0X7e]

# Commandes PRO3
AIGUILLAGE_OFF = 0x60
AIGUILLAGE_ON = 0x61
AIGUILLAGE_FROM = 0x63

# Longueurs commandes PRO
LONGUEUR_PRO1 = 3
LONGUEUR_PRO2 = 4
LONGUEUR_PRO3 = 5

# Autres codes
COPIE_FRANCAIS = 0x6a
COPIE_AMERICAIN = 0x6b
ETEN = 0x41
C0 = 0x43

# Codes PRO2+START/STOP
ROULEAU = 0x43
PROCEDURE = 0x44
MINUSCULES = 0x45

# Codes PRO2+PROG
B9600 = 0x7f
B4800 = 0x76
B1200 = 0x64
B300 = 0x52

# Codes PRO3+START/STOP
ETEN = 0x41
C0 = 0x43

# Codes de réception
RCPT_ECRAN = 0x58
RCPT_CLAVIER = 0x59
RCPT_MODEM = 0x5a
RCPT_PRISE = 0x5b

# Codes d’émission
EMET_ECRAN = 0x50
EMET_CLAVIER = 0x51
EMET_MODEM = 0x52
EMET_PRISE = 0x53

# Accents
ACCENT_CEDILLE = [SS2, 0x4b]
ACCENT_GRAVE = [SS2, 0x41]
ACCENT_AIGU = [SS2, 0x42]
ACCENT_CIRCONFLEXE = [SS2, 0x43]
ACCENT_TREMA = [SS2, 0x48]

# Touches de direction
HAUT = [CSI, 0x41]
BAS = [CSI, 0x42]
GAUCHE = [CSI, 0x44]
DROITE = [CSI, 0x43]

MAJ_HAUT = [CSI, 0x4D]
MAJ_BAS = [CSI, 0x4C]
MAJ_GAUCHE = [CSI, 0x50]
MAJ_DROITE = [CSI, 0x34, 0x68]

CTRL_GAUCHE = 0x7f

# Touche Entrée/Retour chariot
ENTREE      = 0x0d
MAJ_ENTREE  = [CSI, 0x48]
CTRL_ENTREE = [CSI, 0x32, 0x4a]

# Touches de fonction
ENVOI      = [DC3, 0x41]
RETOUR     = [DC3, 0x42]
REPETITION = [DC3, 0x43]
GUIDE      = [DC3, 0x44]
ANNULATION = [DC3, 0x45]
SOMMAIRE   = [DC3, 0x46]
CORRECTION = [DC3, 0x47]
SUITE      = [DC3, 0x48]
CONNEXION  = [DC3, 0x49]

# Types de minitels
TYPE_MINITELS = {
    'b': {
        'nom': 'Minitel 1',
        'retournable': False,
        'clavier': 'ABCD',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'c': {
        'nom': 'Minitel 1',
        'retournable': False,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False 
    },
    'd': {
        'nom': 'Minitel 10',
        'retournable': False,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'e': {
        'nom': 'Minitel 1 couleur',
        'retournable': False,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'f': {
        'nom': 'Minitel 10',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'g': {
        'nom': 'Émulateur',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 9600,
        '80colonnes': True,
        'caracteres': True
    },
    'j': {
        'nom': 'Imprimante',
        'retournable': False,
        'clavier': None,
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'r': {
        'nom': 'Minitel 1',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    's': {
        'nom': 'Minitel 1 couleur',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False 
    },
    't': {
        'nom': 'Terminatel 252',
        'retournable': False,
        'clavier': None,
        'vitesse': 1200,
        '80colonnes': False,
        'caracteres': False
    },
    'u': {
        'nom': 'Minitel 1B',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 4800,
        '80colonnes': True,
        'caracteres': False
    },
    'v': {
        'nom': 'Minitel 2',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 9600,
        '80colonnes': True,
        'caracteres': True
    },
    'w': {
        'nom': 'Minitel 10B',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 4800,
        '80colonnes': True,
        'caracteres': False
    },
    'y': {
        'nom': 'Minitel 5',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 9600,
        '80colonnes': True,
        'caracteres': True
    },
    'z': {
        'nom': 'Minitel 12',
        'retournable': True,
        'clavier': 'Azerty',
        'vitesse': 9600,
        '80colonnes': True,
        'caracteres': True
    },
}

# Les niveaux de gris s’échelonnent comme suit :
# nor, bleu, rouge, magenta, vert, cyan, jaune, blanc
COULEURS_MINITEL = {
    'noir': 0, 'rouge': 1, 'vert': 2, 'jaune': 3,
    'bleu': 4, 'magenta': 5, 'cyan': 6, 'blanc': 7,
    '0': 0, '1': 4, '2': 1, '3': 5,
    '4': 2, '5': 6, '6': 3, '7': 7,
    0: 0, 1: 4, 2: 1, 3: 5,
    4: 2, 5: 6, 6: 3, 7: 7
}

# Capacités les plus basiques du Minitel
CAPACITES_BASIQUES = {
    'nom': 'Minitel inconnu',
    'retournable': False,
    'clavier': 'ABCD',
    'vitesse': 1200,
    'constructeur': 'Inconnu',
    '80colonnes': False,
    'caracteres': False,
    'version': None
}

# Codes d’identification du constructeur
CONSTRUCTEURS = {
    'A': 'Matra',
    'B': 'RTIC',
    'C': 'Telic-Alcatel',
    'D': 'Thomson',
    'E': 'CCS',
    'F': 'Fiet',
    'G': 'Fime',
    'H': 'Unitel',
    'I': 'Option',
    'J': 'Bull',
    'K': 'Télématique',
    'L': 'Desmet'
}
