#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Classe de gestion de menu"""
from .UI import UI
from ..constantes import HAUT, BAS
from ..Sequence import Sequence

class Menu(UI):
    """Classe de gestion de menu

    Cette classe permet d’afficher un menu à l’utilisateur afin qu’il puisse
    sélectionner une entrée via les touches HAUT et BAS.

    La gestion de l’action en cas de validation ou d’annulation est à la charge
    du programme appelant.

    Elle instaure les attributs suivants :

    - options : tableau contenant les options (des chaînes unicodes),
    - selection : option sélectionnée (index dans le tableau d’options),
    - largeur_ligne : largeur de ligne déterminée à partir de la ligne la plus
      longue.

    Les options sont contenues dans un tableau de la forme suivante::

        options = [
          u'Nouveau',
          u'Ouvrir',
          u'-',
          u'Enregistrer',
          u'Enreg. sous...',
          u'Rétablir',
          u'-',
          u'Aperçu',
          u'Imprimer...',
          u'-',
          u'Fermer',
          u'Quitter'
        ]

    Un moins (-) indique un séparateur.

    Il ne peut pas y avoir 2 fois la même entrée dans la liste d’options.

    """
    def __init__(self, minitel, options, posx, posy, selection = 0,
                 couleur = None, grille = True):
        self.options = options
        self.selection = selection
        self.grille = grille

        # Détermine la largeur du menu
        self.largeur_ligne = 0
        for option in self.options:
            self.largeur_ligne = max(self.largeur_ligne, len(option))

        # Détermine la largeur et la hauteur de la zone d’affichage du menu
        largeur = self.largeur_ligne + 2
        hauteur = len(self.options) + 2

        UI.__init__(self, minitel, posx, posy, largeur, hauteur, couleur)

        self.activable = True

    def gere_touche(self, sequence):
        """Gestion des touches

        Cette méthode est appelée automatiquement par la méthode executer.

        Les touches gérées par la classe ChampTexte sont HAUT et BAS pour se
        déplacer dans le menu.

        Un bip est émis si on appuie sur la touche HAUT (respectivement BAS)
        alors que la sélection est déjà sur la première (respectivement
        dernière) ligne.

        :param sequence:
            La séquence reçue du Minitel.
        :type sequence:
            un objet Sequence

        :returns:
            True si la touche a été gérée par le champ texte, False sinon.
        """
        assert isinstance(sequence, Sequence)

        if sequence.egale(HAUT):
            selection = self.option_precedente(self.selection)
            if selection == None:
                self.minitel.bip()
            else:
                self.change_selection(selection)

            return True

        if sequence.egale(BAS):
            selection = self.option_suivante(self.selection)
            if selection == None:
                self.minitel.bip()
            else:
                self.change_selection(selection)

            return True

        return False

    def affiche(self):
        """Affiche le menu complet"""
        i = 0

        # Ligne du haut
        if self.grille:
            # Position sur la ligne du haut
            self.minitel.position(self.posx + 1, self.posy)

            # Application de la couleur si besoin
            if self.couleur != None:
                self.minitel.couleur(caractere = self.couleur)

            # Trace la ligne du haut
            self.minitel.repeter(0x5f, self.largeur_ligne)

        # Trace les lignes une par une
        for _ in self.options:
            # Application de la couleur si besoin
            if self.couleur != None:
                self.minitel.couleur(caractere = self.couleur)

            # Affiche la ligne courante en indiquant si elle est sélectionnée
            self.affiche_ligne(i, self.selection == i)
            i += 1

        # Ligne du bas
        if self.grille:
            # Position sur la ligne du bas
            self.minitel.position(self.posx + 1, self.posy + len(self.options) + 1)

            # Application de la couleur si besoin
            if self.couleur != None:
                self.minitel.couleur(caractere = self.couleur)

            # Trace la ligne du bas
            self.minitel.repeter(0x7e, self.largeur_ligne)

    def affiche_ligne(self, selection, etat = False):
        """Affiche une ligne du menu
        
        :param selection:
            index de la ligne à afficher dans la liste des options
        :type selection:
            un entier positif
        
        :param etat:
        :type etat:
            un booléen
        """
        assert isinstance(selection, int)
        assert selection >= 0 and selection < len(self.options)
        assert etat in [True, False]

        # Positionne au début de la ligne
        self.minitel.position(self.posx, self.posy + selection + 1)

        # Application de la couleur si besoin
        if self.couleur != None:
            self.minitel.couleur(caractere = self.couleur)

        # Dessine la ligne gauche
        if self.grille:
            self.minitel.envoyer([0x7d])

        # 2 cas possibles : un séparateur ou une entrée normale
        if self.options[selection] == '-':
            if self.grille:
                self.minitel.repeter(0x60, self.largeur_ligne)
        else:
            # Si l’option est sélectionnée, on applique l’effet vidéo inverse
            if etat:
                self.minitel.effet(inversion = True)

            # Dessine une entrée en la justifiant à gauche sur la largeur de
            # la ligne
            option = self.options[selection]
            while len(option) < self.largeur_ligne:
                option = option + ' '
            self.minitel.envoyer(option)

        # Si l’option est sélectionnée, on arrête l’effet vidéo inverse
        if etat:
            self.minitel.effet(inversion = False)

        # Dessine la ligne droite
        if self.grille:
            self.minitel.envoyer([0x7b])
        
    def change_selection(self, selection):
        """Change la sélection en cours
        
        :param selection:
            index de la ligne à sélectionner dans la liste des options
        :type selection:
            un entier positif
        """
        assert isinstance(selection, int)
        assert selection >= 0 and selection < len(self.options)

        # Si la nouvelle sélection est la sélection courante, on ignore
        if self.selection == selection:
            return

        # Affiche la ligne sélectionnée actuelle comme non sélectionnée
        self.affiche_ligne(self.selection, False)

        # Affiche la nouvelle ligne sélectionnée
        self.affiche_ligne(selection, True)

        # Met à jour l’index de l’option sélectionnée
        self.selection = selection

    def option_suivante(self, numero):
        """Détermine l’index de l’option suivante

        Retourne l’index de l’option suivant l’option indiqué par l’argument
        numero.

        :param numero:
            index de la ligne à sélectionner dans la liste des options
        :type numero:
            un entier positif

        :returns:
            l’index de l’option suivante ou None si elle n’existe pas.
        """
        assert isinstance(numero, int)
        assert numero >= 0 and numero < len(self.options)

        # Parcours les options après l’index numéro en restant dans les limites
        # de la liste des options
        for i in range(numero + 1, len(self.options)):
            # Les séparateurs sont ignorés
            if self.options[i] != '-':
                return i

        # Aucune option n’a été trouvée après celle indiquée dans numero
        return None
    
    def option_precedente(self, numero):
        """Détermine l’index de l’option précédente

        Retourne l’index de l’option précédant l’option indiqué par l’argument
        numero.

        :param numero:
            index de la ligne à sélectionner dans la liste des options
        :type numero:
            un entier positif

        :returns:
            l’index de l’option précédente ou None si elle n’existe pas.
        """
        assert isinstance(numero, int)
        assert numero >= 0 and numero < len(self.options)

        # Parcours les options avant l’index numéro en restant dans les limites
        # de la liste des options
        for i in range(numero - 1, -1, -1):
            # Les séparateurs sont ignorés
            if self.options[i] != '-':
                return i

        # Aucune option n’a été trouvée avant celle indiquée dans numero
        return None
