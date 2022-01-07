#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Classe de gestion de champ texte"""

from .UI import UI
from ..constantes import (
    GAUCHE, DROITE, CORRECTION, ANNULATION, ACCENT_AIGU, ACCENT_GRAVE, ACCENT_CIRCONFLEXE, 
    ACCENT_TREMA, ACCENT_CEDILLE
)

# Caractères en provenance du Minitel gérés par le champ texte
CARACTERES_MINITEL = (
    'abcdefghijklmnopqrstuvwxyz' +
    'ABCDEFGHIJKLMNOPQRSTUVWXYZ' +
    ' *$!:;,?./&(-_)=+\'@#' +
    '0123456789'
)

class ChampTexte(UI):
    """Classe de gestion de champ texte

    Cette classe gère un champ texte. À l’instar des champs texte d’un
    formulaire HTML, ce champ texte dispose d’une longueur affichable et d’une
    longueur totale maxi.

    ChampTexte ne gère aucun label.

    Les attributs suivants sont disponibles :

    - longueur_visible : longueur occupé par le champ à l’écran
    - longueur_totale : nombre de caractères maximum du champ
    - valeur : valeur du champ (encodée UTF-8)
    - curseur_x : position du curseur dans le champ
    - decalage : début d’affichage du champ à l’écran
    - accent : accent en attente d’application sur le prochain caractère
    - champ_cache : les caractères ne sont pas affichés sur le minitel, il sont
                    remplacés par des '*' (utiliser pour les mots de passes par exemple)
    """
    def __init__(self, minitel, posx, posy, longueur_visible,
                 longueur_totale = None, valeur = '', couleur = None, champ_cache=False):
        assert isinstance(posx, int)
        assert isinstance(posy, int)
        assert isinstance(longueur_visible, int)
        assert isinstance(longueur_totale, int) or longueur_totale == None
        assert isinstance(valeur, str)
        assert posx + longueur_visible < 80
        assert longueur_visible >= 1
        if longueur_totale == None:
            longueur_totale = longueur_visible
        assert longueur_visible <= longueur_totale

        UI.__init__(self, minitel, posx, posy, longueur_visible, 1, couleur)

        # Initialise le champ
        self.longueur_visible = longueur_visible
        self.longueur_totale = longueur_totale
        self.valeur = '' + valeur
        self.curseur_x = 0
        self.decalage = 0
        self.activable = True
        self.accent = None
        self.champ_cache = champ_cache

    def gere_touche(self, sequence):
        """Gestion des touches

        Cette méthode est appelée automatiquement par la méthode executer.

        Les touches gérées par la classe ChampTexte sont les suivantes :

        - GAUCHE, DROITE, pour se déplacer dans le champ,
        - CORRECTION, pour supprimer le caractère à gauche du curseur,
        - ANNULATION, pour supprimer tout le champ,
        - ACCENT_AIGU, ACCENT_GRAVE, ACCENT_CIRCONFLEXE, ACCENT_TREMA,
        - ACCENT_CEDILLE,
        - les caractères de la norme ASCII pouvant être tapés sur un clavier
          de Minitel.

        :param sequence:
            La séquence reçue du Minitel.
        :type sequence:
            un objet Sequence

        :returns:
            True si la touche a été gérée par le champ texte, False sinon.
        """
        if sequence.egale(GAUCHE):
            self.accent = None
            self.curseur_gauche()
            return True        
        elif sequence.egale(DROITE):
            self.accent = None
            self.curseur_droite()
            return True        
        elif sequence.egale(CORRECTION):
            self.accent = None
            if self.curseur_gauche():
                self.valeur = (self.valeur[0:self.curseur_x] +
                               self.valeur[self.curseur_x + 1:])
                self.affiche()
            return True
        elif sequence.egale(ANNULATION):
            self.accent = None
            self.valeur = ''
            self.curseur_x = 0
            self.affiche()
            return True
        elif (sequence.egale(ACCENT_AIGU) or
              sequence.egale(ACCENT_GRAVE) or
              sequence.egale(ACCENT_CIRCONFLEXE) or
              sequence.egale(ACCENT_TREMA)):
            self.accent = sequence
            return True
        elif sequence.egale([ACCENT_CEDILLE, 'c']):
            self.accent = None
            self.valeur = (self.valeur[0:self.curseur_x] +
                           'ç' +
                           self.valeur[self.curseur_x:])
            self.curseur_droite()
            self.affiche()
            return True
        elif chr(sequence.valeurs[0]) in CARACTERES_MINITEL:
            caractere = '' + chr(sequence.valeurs[0])
            if self.accent != None:
                if caractere in 'aeiou':
                    if self.accent.egale(ACCENT_AIGU):
                        caractere = 'áéíóú'['aeiou'.index(caractere)]
                    elif self.accent.egale(ACCENT_GRAVE):
                        caractere = 'àèìòù'['aeiou'.index(caractere)]
                    elif self.accent.egale(ACCENT_CIRCONFLEXE):
                        caractere = 'âêîôû'['aeiou'.index(caractere)]
                    elif self.accent.egale(ACCENT_TREMA):
                        caractere = 'äëïöü'['aeiou'.index(caractere)]

                self.accent = None

            self.valeur = (self.valeur[0:self.curseur_x] +
                           caractere +
                           self.valeur[self.curseur_x:])
            self.curseur_droite()
            self.affiche()
            return True        

        return False

    def curseur_gauche(self):
        """Déplace le curseur d’un caractère sur la gauche

        Si le curseur ne peut pas être déplacé, un bip est émis.

        Si le curseur demande à se déplacer dans une partie du champ non
        encore visible, un décalage s’opère.

        :returns:
            True si le curseur a subi un déplacement, False sinon.
        """
        # On ne peut déplacer le curseur à gauche s’il est déjà sur le premier
        # caractère
        if self.curseur_x == 0:
            self.minitel.bip()
            return False

        self.curseur_x = self.curseur_x - 1

        # Effectue un décalage si le curseur déborde de la zone visible
        if self.curseur_x < self.decalage:
            self.decalage = max(
                0,
                int(self.decalage - self.longueur_visible / 2)
            )
            self.affiche()
        else:
            self.minitel.position(
                self.posx + self.curseur_x - self.decalage,
                self.posy
            )

        return True
    
    def curseur_droite(self):
        """Déplace le curseur d’un caractère sur la droite

        Si le curseur ne peut pas être déplacé, un bip est émis.

        Si le curseur demande à se déplacer dans une partie du champ non
        encore visible, un décalage s’opère.

        :returns:
            True si le curseur a subi un déplacement, False sinon.
        """
        # On ne peut déplacer le curseur à droite s’il est déjà sur le dernier
        # caractère ou à la longueur max
        if self.curseur_x == min(len(self.valeur), self.longueur_totale):
            self.minitel.bip()
            return False
    
        self.curseur_x = self.curseur_x + 1

        # Effectue un décalage si le curseur déborde de la zone visible
        if self.curseur_x > self.decalage + self.longueur_visible:
            self.decalage = max(
                0,
                int(self.decalage + self.longueur_visible / 2)
            )
            self.affiche()
        else:
            self.minitel.position(
                self.posx + self.curseur_x - self.decalage,
                self.posy
            )

        return True

    def gere_arrivee(self):
        """Gère l’activation du champ texte

        La méthode place positionne le curseur et le rend visible.
        """
        self.minitel.position(
            self.posx + self.curseur_x - self.decalage,
            self.posy
        )
        self.minitel.curseur(True)

    def gere_depart(self):
        """Gère la désactivation du champ texte

        La méthode annule tout début d’accent et rend invisible le curseur.
        """
        self.accent = None
        self.minitel.curseur(False)

    def affiche(self):
        """Affiche le champ texte

        Si la valeur est plus petite que la longueur affichée, on remplit les
        espaces en trop par des points.

        Après appel à cette méthode, le curseur est automatiquement positionné.

        Cette méthode est appelée dès que l’on veut afficher l’élément.
        """
        # Début du champ texte à l’écran
        self.minitel.curseur(False)
        self.minitel.position(self.posx, self.posy)

        # Couleur du label
        if self.couleur != None:
            self.minitel.couleur(caractere = self.couleur)

        if not self.champ_cache :
            #Si le champ n'est pas caché, on affiche les caractères
            val = str( self.valeur )
        else : 
            val = "*" * len( self.valeur  ) 

        if len(val) - self.decalage <= self.longueur_visible:
            # Cas valeur plus petite que la longueur visible
            affichage = val[self.decalage:]
#             affichage = affichage.ljust(self.longueur_visible, '.')
            while len(affichage) < self.longueur_visible:
                affichage = affichage + '.'
        else:
            # Cas valeur plus grande que la longueur visible
            affichage = val[
                self.decalage:
                self.decalage + self.longueur_visible
            ]

        # Affiche le contenu
        self.minitel.envoyer(affichage)

        # Place le curseur visible
        self.minitel.position(
            self.posx + self.curseur_x - self.decalage,
            self.posy
        )
        self.minitel.curseur(True)

