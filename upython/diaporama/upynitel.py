#!/usr/bin/env python3

# ré-écritude en python3 de Cristel/Dragster !
# (C) 1984-2017 Christian Quest / A-GPL

import time


class Pynitel:
    "Classe de gestion des entrée/sortie vidéotex avec un Minitel"

    def __init__(self, conn):
        self.ecrans = {'last': None}
        self.conn = conn
        self.lastkey = 0
        self.lastscreen = ''
        self.laststar = False
        self.zones = []
        self.zonenumber = 0

        # constantes de couleurs
        self.noir = 0
        self.rouge = 1
        self.vert = 2
        self.jaune = 3
        self.bleu = 4
        self.magenta = 5
        self.cyan = 6
        self.blanc = 7

        # constantes des touches de fonction du Minitel
        # en mode Vidéotex ou Mixte
        self.envoi = 1
        self.retour = 2
        self.repetition = 3
        self.guide = 4
        self.annulation = 5
        self.sommaire = 6
        self.correction = 7
        self.suite = 8
        self.connexionfin = 9

        # constantes des séquences protocole
        self.PRO1 = '\x1b\x39'
        self.PRO2 = '\x1b\x3a'
        self.PRO3 = '\x1b\x3b'

    def wait(self):
        "Attente d'une connexion"

        print('ATTENTE')

        # ESC reçu... on considère qu'on est connecté
        while self.conn.read(1) != b' ':
            time.sleep(1)

        print('CONNECTION')

    def end(self):
        "Fin de connexion, raccrochage"
        self.conn.write(b'\x1b9g')

    def _if(self):
        "Dernier caractère reçu"
        data = self.conn.read()
        if not data:
            return None
        else:
            return data

#     def clear(self):
#         "Efface le buffer de réception"
#         self.conn.settimeout(0)  # timeout de 2 minutes pour les saisies...
#         self.conn.recv(10000)

    def home(self):
        "Efface écran et ligne 0"
        self._del(0, 1)
        self.sendchr(12)  # FF
        self.cursor(False)  # Coff
        print('home')

    def vtab(self, ligne):
        "Positionne le curseur sur un début de ligne"
        self.pos(ligne, 1)

    def pos(self, ligne, colonne=1):
        "Positionne le curseur sur une ligne / colonne"
        if ligne == 1 and colonne == 1:
            self.sendchr(30)
        else:
            self.sendchr(31)
            self.sendchr(64+ligne)
            self.sendchr(64+colonne)

    def _del(self, ligne, colonne):
        "Effacement jusque fin de ligne"
        self.pos(ligne, colonne)
        self.sendchr(24)

    def normal(self):
        "Passage en vidéo normale"
        self.sendesc('I')

    def backcolor(self, couleur):
        """Change la couleur de fond,
        à valider par un espace pour le texte (identique à HCOLOR)"""
        self.sendesc(chr(80+couleur))

    def canblock(self, debut, fin, colonne, inverse=False):
        """Efface un rectangle sur l'écran,
        compris entre deux lignes et après une colonne"""
        if inverse is False:
            self.pos(debut, colonne)
            self.sendchr(24)
            for ligne in range(debut, fin):
                self.sendchr(10)
                self.sendchr(24)
        else:
            self.pos(fin, colonne)
            self.sendchr(24)
            for ligne in range(debut, fin):
                self.sendchr(11)
                self.sendchr(24)

    def caneol(self, ligne, colonne):
        "Efface la fin de ligne derrière la colonne spécifiée"
        self.pos(ligne, colonne)
        self.sendchr(24)  # CAN

    def cls(self):
        "Efface l'écran du Minitel"
        self.home()

    def color(self, couleur):
        "Change la couleur du texte ou graphique"
        self.sendesc(chr(64+couleur))

    # curpos - donne la position actuelle du curseur du Minitel
    def cursor(self, visible):
        "Permet de rendre apparent ou invisible le curseur clignotant"
        if visible == 1 or visible is True:
            self.sendchr(17)  # Con
        else:
            self.sendchr(20)  # Coff

    # dial - appel un numéro de téléphone
    def draw(self, num=0):
        "Envoi un écran préchargé dans un buffer vers le minitel"
        if num is None:
            num = self.ecrans['last']
        self.ecrans['last'] = num
        if num is not None:
            self.conn.write(self.ecrans[num])

    def drawscreen(self, fichier):
        "Envoi du contenu d'un fichier"
        with open(fichier, 'rb') as f:
            self.conn.write(f.read())

    def flash(self, clignote=True):
        "Passage en clignotant"
        if clignote is None or clignote is True or clignote == 1:
            self.sendesc('\x48')
        else:
            self.sendesc('\x49')

    def forecolor(self, couleur):
        "Change la couleur des caractères"
        self.color(couleur)

    def get(self):
        "Rend le contenu du buffer de saisie actuel"
        return(self.conn.read(self.conn.any()).decode())

    # getid - lecture ROM/RAM Minitel
    def getid(self):
        print("getid: non implémenté...")
        return

    def hcolor(self, couleur):
        "Change la couleur de fond, à valider par un espace pour le texte"
        self.sendesc(chr(80+couleur))

    def input(self, ligne, colonne, longueur, data='',
              caractere='.', redraw=True):
        "Gestion de zone de saisie"
        # affichage initial
        if redraw:
            self.sendchr(20)  # Coff
            self.pos(ligne, colonne)
            self._print(data)
            self.plot(caractere, longueur-len(data))
        self.pos(ligne, colonne+len(data))
        self.sendchr(17)  # Con

        while True:
            if self.conn.any():
                c = self.conn.read(1).decode()
            else:
                c = ''
            if c == '':
                continue
            elif c == '\x13':  # SEP donc touche Minitel...
                c = self.conn.read(1).decode()

                if c == '\x45' and data != '':  # annulation
                    data = ''
                    self.sendchr(20)  # Coff
                    self.pos(ligne, colonne)
                    self._print(data)
                    self.plot(caractere, longueur-len(data))
                    self.pos(ligne, colonne)
                    self.sendchr(17)  # Con
                elif c == '\x47' and data != '':  # correction
                    self.send(chr(8)+caractere+chr(8))
                    data = data[:len(data)-1]
                else:
                    self.lastkey = ord(c)-64
                    self.laststar = (data != '' and data[:-1] == '*')
                    return(data, ord(c)-64)
            elif c == '\x1b':  # filtrage des acquittements protocole...
                c = c + self.conn.read(1).decode()
                if c == self.PRO1:
                    self.conn.read(1)
                elif c == self.PRO2:
                    self.conn.read(2)
                elif c == self.PRO3:
                    self.conn.read(3)
            elif c >= ' ' and len(data) >= longueur:
                self.bip()
            elif c >= ' ':
                data = data + c

    def inverse(self, inverse=1):
        "Passage en inverse"
        if inverse is None or inverse == 1 or inverse is True:
            self.sendesc('\x5D')
        else:
            self.sendesc('\x5C')

    def locate(self, ligne, colonne):
        "Positionne le curseur"
        self.pos(ligne, colonne)

    # lower - clavier en mode minuscule / majuscule (mode "Enseignement")
    def lower(self, islower=True):
        if islower or islower == 1:
            self.send(self.PRO2+'\x69\x45')  # passage clavier en minuscules
        else:
            self.send(self.PRO2+'\x6a\x45')  # retour clavier majuscule

    def message(self, ligne, colonne, delai, message, bip=False):
        """Affiche un message à une position donnée pendant un temps donné,
        puis l'efface"""
        if bip:
            self.bip()
        self.pos(ligne, colonne)
        self._print(message)
        self.conn.read(self.conn.any())
        time.sleep(delai)
        self.pos(ligne, colonne)
        self.plot(' ', len(message))

    def printscreen(self, fichier):
        self.drawscreen(fichier)

    def resetzones(self):
        while len(self.zones) > 0:
            self.zones.pop()

    # scroll - Active ou désactive le mode "rouleau"

    def starflag(self):
        """Indique si la dernière saisie s'est terminée par une étoile
        + touche de fonction"""
        return(self.laststar)

    # status - Etat du modem

    # swmodem - Retournement du modem

    # sysparm - Paramètres du modem

    def underline(self, souligne=True):
        "Passe en mode souligné ou normal"
        if souligne is None or souligne is True or souligne == 1:
            self.sendesc(chr(90))
        else:
            self.sendesc(chr(89))

    def waitzones(self, zone):
        "Gestion de zones de saisie"
        if len(self.zones) == 0:
            return (0, 0)

        zone = -zone

        while True:
            # affichage initial
            if zone <= 0:
                self.cursor(False)
                for z in self.zones:
                    self.pos(z['ligne'], z['colonne'])
                    if z['couleur'] != self.blanc:
                        self.forecolor(z['couleur'])
                    self._print(z['texte'])
                if zone < 0:
                    zone = -zone

            # gestion de la zone de saisie courante
            (self.zones[zone-1]['texte'], touche) = self.input(self.zones[zone-1]['ligne'],  # noqa
                self.zones[zone-1]['colonne'], self.zones[zone-1]['longueur'],
                data=self.zones[zone-1]['texte'], caractere='.', redraw=False)

            # gestion des SUITE / RETOUR
            if touche == self.suite:
                if zone < len(self.zones):
                    zone = zone+1
                else:
                    zone = 1
            elif touche == self.retour:
                if zone > 1:
                    zone = zone-1
                else:
                    zone = len(self.zones)
            else:
                self.zonenumber = zone
                self.cursor(False)
                return(zone, touche)

    # waitconnect - attente de CONNECTION

    def zone(self, ligne, colonne, longueur, texte, couleur):
        "Déclaration d'une zone de saisie"
        self.zones.append({"ligne": ligne, "colonne": colonne,
                           "longueur": longueur, "texte": texte,
                           "couleur": couleur})

    def key(self):
        "Dernière touche de fonction utilisée sur le Minitel lors d'une saisie"
        return self.lastkey

    def scale(self, taille):
        "Change la taille du texte"
        self.sendesc(chr(76+taille))

    def notrace(self):
        "Passe en texte souligné, à valider par un espace"
        self.sendesc(chr(89))

    def trace(self):
        "Fin de texte souligné, à valider par un espace"
        self.sendesc(chr(90))

    def plot(self, car, nombre):
        "Affichage répété d'un caractère"
        if nombre > 1:
            self._print(car)
        if nombre == 2:
            self._print(car)
        elif nombre > 2:
            while nombre > 63:
                self.sendchr(18)
                self.sendchr(64+63)
                nombre = nombre-63
            self.sendchr(18)
            self.sendchr(64+nombre-1)

    def text(self):
        "Mode texte"
        self.sendchr(15)

    def gr(self):
        "Mode graphique"
        self.sendchr(14)

    def step(self, scroll):
        "Active ou désactive le mode scrolling"
        self.sendesc(':')
        self.sendchr(ord('j')-scroll)
        self.send('C')

    def xdraw(self, fichier):
        "Envoi du contenu d'un fichier"
        with open(fichier, 'rb') as f:
            self.conn.write(f.read())

    def load(self, num, fichier):
        "Charge un fichier vidéotex dans un buffer"
        with open(fichier, 'rb') as f:
            data = f.read()
            self.ecrans[num] = data

    def read(self):
        "Lecture de la date et heure"
        print('read: non implémenté')

    def _print(self, texte):
        self.send(self.accents(texte))

    def send(self, text):
        "Envoi de données vers le minitel"
        if self.conn is not None:
            self.conn.write(text.encode())
        else:
            print('conn = None')

    def sendchr(self, ascii):
        self.send(chr(ascii))

    def sendesc(self, text):
        self.sendchr(27)
        self.send(text)

    def bip(self):
        self.sendchr(7)

    def accents(self, text):
        "Conversion des caractères accentués (cf STUM p 103)"
        text = text.replace('à', '\x19\x41a')
        text = text.replace('â', '\x19\x43a')
        text = text.replace('ä', '\x19\x48a')
        text = text.replace('è', '\x19\x41e')
        text = text.replace('é', '\x19\x42e')
        text = text.replace('ê', '\x19\x43e')
        text = text.replace('ë', '\x19\x48e')
        text = text.replace('î', '\x19\x43i')
        text = text.replace('ï', '\x19\x48i')
        text = text.replace('ô', '\x19\x43o')
        text = text.replace('ö', '\x19\x48o')
        text = text.replace('ù', '\x19\x43u')
        text = text.replace('û', '\x19\x43u')
        text = text.replace('ü', '\x19\x48u')
        text = text.replace('ç', '\x19\x4Bc')
        text = text.replace('°', '\x19\x30')
        text = text.replace('£', '\x19\x23')
        text = text.replace('Œ', '\x19\x6A').replace('œ', '\x19\x7A')
        text = text.replace('ß', '\x19\x7B')

        # Caractères spéciaux
        text = text.replace('¼', '\x19\x3C')
        text = text.replace('½', '\x19\x3D')
        text = text.replace('¾', '\x19\x3E')
        text = text.replace('←', '\x19\x2C')
        text = text.replace('↑', '\x19\x2D')
        text = text.replace('→', '\x19\x2E')
        text = text.replace('↓', '\x19\x2F')
        text = text.replace('̶', '\x60')
        text = text.replace('|', '\x7C')

        # Caractères accentués inexistants sur Minitel
        text = text.replace('À', 'A').replace('Â', 'A').replace('Ä', 'A')
        text = text.replace('È', 'E').replace('É', 'E')
        text = text.replace('Ê', 'E').replace('Ë', 'E')
        text = text.replace('Ï', 'I').replace('Î', 'I')
        text = text.replace('Ô', 'O').replace('Ö', 'O')
        text = text.replace('Ù', 'U').replace('Û', 'U').replace('Ü', 'U')
        text = text.replace('Ç', 'C')

        return(text)
