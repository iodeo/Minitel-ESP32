#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Classe de gestion de label"""
from .UI import UI

class Label(UI):
    """Classe de gestion de label

    Elle ne fait qu’afficher un texte d’une seule ligne.
    """
    def __init__(self, minitel, posx, posy, valeur = '', couleur = None):
        assert isinstance(posx, int)
        assert isinstance(posy, int)
        assert isinstance(valeur, str) or isinstance(valeur, str)

        # Initialise le champ
        self.valeur = valeur

        UI.__init__(self, minitel, posx, posy, len(self.valeur), 1, couleur)

    def gere_touche(self, sequence):
        """Gestion des touches

        Cette méthode est appelée automatiquement par la méthode executer.

        Un Label ne gère aucune touche et renvoie donc tout le temps False.

        :param sequence:
            La séquence reçue du Minitel.
        :type sequence:
            un objet Sequence

        :returns:
            False
        """
        return False

    def affiche(self):
        """Affiche le label

        Cette méthode est appelée dès que l’on veut afficher l’élément.
        """
        # Début du label à l’écran
        self.minitel.position(self.posx, self.posy)

        # Couleur du label
        if self.couleur != None:
            self.minitel.couleur(caractere = self.couleur)

        # Affiche le contenu
        self.minitel.envoyer(self.valeur)

