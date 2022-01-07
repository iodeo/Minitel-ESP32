#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Fonctions outils supplémentaires
"""

from minitel.constantes import COULEURS_MINITEL

def affiche_videotex(minitel, fichier):
    """ Fonction permettant d'afficher un fichier videotex (.vdt .vtx)
    
    :param minitel:
        L’objet auquel envoyer les commandes et recevoir les appuis de
        touche.
    :type minitel:
        un objet Minitel
    
    :param fichier:
        Le chemin du fichier à afficher
    :type fichier:
        une chaine de caractere
    """
    with open(fichier, 'rb') as f:
        minitel.envoyer_brut(f.read())

def traine_caractere(minitel, x0, x1, y, car, couleur = None, fond = None):
    """ Fonction animation permettant de faire glisser une chaine de caractere
    horizontalement en avant ou en arrière
    
    :param minitel:
        L’objet auquel envoyer les commandes et recevoir les appuis de
        touche.
    :type minitel:
        un objet Minitel

    :param x0:
        Coordonnée de départ du premier caractère de la chaine
    :type x0:
        un entier
        
    :param x1:
        Coordonnée d'arrivée du premier caractère de la chaine
    :type x1:
        un entier
    
    :param y:
        Coordonnée y de la chaine de caractere
    :type y:
        un entier
    
    :param car:
        La chaine de caractere à afficher
    :type car:
        un caractere ou une chaine de caractere
    
    :param couleur:
        la couleur du caractere
    :type couleur:
        une chaine de caractere correspondant à une entree de COULEURS_MINITEL
    
    :param fond:
        la couleur du fond
    :type fond:
        une chaine de caractere correspondant à une entree de COULEURS_MINITEL
    """
    minitel.position(x0,y)
    if couleur:
        minitel.couleur(caractere = couleur)
    if fond:
        minitel.couleur(fond = fond)
    minitel.envoyer(car)
    if x0 < x1:
        while x0 < x1:
            minitel.position(x0,y)
            if couleur:
                minitel.couleur(caractere = couleur)
            if fond:
                minitel.couleur(fond = fond)
            minitel.envoyer(' ' + car)
            x0 = x0+1
    elif x0 > x1:
        while x0 >= x1:
            minitel.position(x0,y)
            if couleur:
                minitel.couleur(caractere = couleur)
            if fond:
                minitel.couleur(fond = fond)
            minitel.envoyer(car + ' ')
            x0 = x0-1
