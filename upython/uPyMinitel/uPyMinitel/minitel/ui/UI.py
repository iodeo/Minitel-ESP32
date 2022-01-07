#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Base pour la création d’une interface utilisateur pour le Minitel"""

# from ..Minitel import Minitel,Empty
from ..Minitel import Minitel

class UI:
    """Classe de base pour la création d’élément d’interface utilisateur

    Cette classe fournit un cadre de fonctionnement pour la création d’autres
    classes pour réaliser une interface utilisateur.

    Elle instaure les attributs suivants :

    - posx et posy : coordonnées haut gauche de l’élément
    - largeur et hauteur : dimensions en caractères de l’élément
    - minitel : un objet Minitel utilisé pour l’affichage de l’élément
    - couleur : couleur d’avant-plan/des caractères
    - activable : booléen indiquant si l’élément peut recevoir les événements
      du Minitel (clavier)

    Les classes dérivées de UI doivent implémenter les méthodes suivantes :
    - __init__ : initialisation de l’objet
    - affiche : affichage de l’objet
    - efface : effacement de l’objet
    - gere_touche : gestion de l’appui d’une touche (si l’élément est activable)
    - gere_arrivee : gestion de l’activation de l’élément
    - gere_depart : gestion de la désactivation de l’élément

    """
    def __init__(self, minitel, posx, posy, largeur, hauteur, couleur):
        """Constructeur

        :param minitel:
            L’objet auquel envoyer les commandes et recevoir les appuis de
            touche.
        :type minitel:
            un objet Minitel

        :param posx:
            Coordonnée x de l’élément
        :type posx:
            un entier

        :param posy:
            Coordonnée y de l’élément
        :type posy:
            un entier
        
        :param largeur:
            Largeur de l’élément en caractères
        :type largeur:
            un entier
        
        :param hauteur:
            Hauteur de l’élément en caractères
        :type hauteur:
            un entier
        
        :param couleur:
            Couleur de l’élément
        :type couleur:
            un entier ou une chaîne de caractères
        """
        assert isinstance(minitel, Minitel)
        assert posx > 0 and posx <= 80
        assert posy > 0 and posy <= 24
        assert largeur > 0 and largeur + posx - 1 <= 80
        assert hauteur > 0 and hauteur + posy - 1 <= 80
        assert isinstance(couleur, (int, str)) or couleur == None

        # Un élément UI est toujours rattaché à un objet Minitel
        self.minitel = minitel

        # Un élément UI occupe une zone rectangulaire de l’écran du Minitel
        self.posx = posx
        self.posy = posy
        self.largeur = largeur
        self.hauteur = hauteur
        self.couleur = couleur

        # Un élément UI peut recevoir ou non les événements clavier
        # Par défaut, il ne les reçoit pas
        self.activable = False

    def executer(self):
        """Boucle d’exécution d’un élément

        L’appel de cette méthode permet de lancer une boucle infinie qui va
        gérer l’appui des touches (méthode gere_touche) provenant du Minitel.
        Dès qu’une touche n’est pas gérée par l’élément, la boucle s’arrête.
        """
        while True:
            try:
                r = self.minitel.recevoir_sequence()
                if not self.gere_touche(r):
                    break
            except AssertionError:
                break

    def affiche(self):
        """Affiche l’élément

        Cette méthode est appelée dès que l’on veut afficher l’élément.
        """
        pass

    def efface(self):
        """Efface l’élément

        Cette méthode est appelée dès que l’on veut effacer l’élément. Par
        défaut elle affiche un rectangle contenant des espaces à la place de
        l’élément. Elle peut être surchargée pour obtenir une gestion plus
        poussée de l’affichage.
        """
        for ligne in range(self.posy, self.posy + self.hauteur):
            self.minitel.position(self.posx, ligne)
            self.minitel.repeter(' ', self.largeur)

    # Désactive un faux positif. Il est normal que cette méthode n’utilise
    # pas l’argument sequence et qu’elle soit une méthode plutôt qu’une
    # fonction
    # pylint: disable-msg=W0613,R0201
    def gere_touche(self, sequence):
        """Gère une touche

        Cette méthode est appelée automatiquement par la méthode executer dès
        qu’une séquence est disponible au traitement.

        Pour tout élément interactif, cette méthode doit être surchargée car
        elle ne traite aucune touche par défaut et renvoie donc False.

        :param sequence:
            la séquence de caractères en provenance du Minitel que l’élément
            doit traiter.
        :type sequence:
            un objet Sequence

        :returns:
            un booléen indiquant si la touche a été prise en charge par
            l’élément (True) ou si l’élément n’a pas pu traitée la touche
            (False).
        """
        return False

    def gere_arrivee(self):
        """Gère l’activation de l’élément

        Cette méthode est appelée lorsque l’élément est activé (lorsqu’il
        reçoit les touches du clavier).
        """
        pass

    def gere_depart(self):
        """Gère la désactivation de l’élément

        Cette méthode est appelée lorsque l’élément est désactivé (lorsqu’il ne
        reçoit plus les touches du clavier).
        """
        pass

