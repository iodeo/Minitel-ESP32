#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Classe permettant de regrouper des éléments d’interface utilisateur"""

from .UI import UI
from ..Sequence import Sequence
from ..constantes import HAUT, BAS, RETOUR, SUITE

class Conteneur(UI):
    """Classe permettant de regrouper des éléments d’interface utilisateur

    Cette classe permet de regrouper des éléments d’inteface utilisateur afin
    de faciliter leur gestion. Elle est notamment capable d’afficher tous les
    éléments qu’elle contient et de gérer le passage d’un élément à un autre.

    Le passage d’un élément à l’autre se fait au moyen de la touche ENTREE pour
    l’élément suivant et de la combinaison MAJUSCULE+ENTREE pour l’élément
    précédent. Si l’utilisateur veut l’élément suivant alors qu’il est déjà
    sur le dernier élément, le Minitel émettra un bip. Idem pour l’élément
    précédent.

    Le éléments dont l’attribut activable est à False sont purement et
    simplement ignorés lors de la navigation inter-éléments.

    Les attributs suivants sont disponibles :

    - elements : liste des éléments dans leur ordre d’apparition
    - element_actif : objet de classe UI désignant l’élément actif
    - fond : couleur de fond du conteneur
    """
    def __init__(self, minitel, posx, posy, largeur, hauteur, couleur = None,
                 fond = None):
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
            un entier, une chaîne de caractères ou None

        :param fond:
            Couleur de fond du conteneur
        :type couleur:
            un entier, une chaîne de caractères ou None
        """
        assert isinstance(posx, int)
        assert isinstance(posy, int)
        assert isinstance(largeur, int)
        assert isinstance(hauteur, int)
        assert isinstance(couleur, (str, int)) or couleur == None
        assert isinstance(fond, (str, int)) or fond == None

        # Initialisation des attributs
        self.elements = []
        self.element_actif = None
        self.fond = fond

        UI.__init__(self, minitel, posx, posy, largeur, hauteur, couleur)

    def gere_touche(self, sequence):
        """Gestion des touches

        Cette méthode est appelée automatiquement par la méthode executer.

        Elle tente avant tout de faire traiter la touche par l’élément actif.
        Si l’élément actif ne gère pas la touche, le conteneur teste si les
        touches ENTREE ou MAJ ENTREE ont été pressées. Ces deux touches
        permettent à l’utilisateur de naviguer entre les éléments.

        En cas de changement d’élément actif, le conteneur appelle la méthode
        gere_depart de l’ancien élément actif et la méthode gere_arrivee du
        nouvel élément actif.

        :param sequence:
            La séquence reçue du Minitel.
        :type sequence:
            un objet Sequence

        :returns:
            True si la touche a été gérée par le conteneur ou l’un de ses
            éléments, False sinon.
        """
        assert isinstance(sequence, Sequence)

        # Aucun élement actif ? Donc rien à faire
        if self.element_actif == None:
            return False

        # Fait suivre la séquence à l’élément actif
        touche_geree = self.element_actif.gere_touche(sequence)

        # Si l’élément actif a traité la séquence, c’est fini
        if touche_geree:
            return True

        # Si l’élément actif n’a pas traité la séquence, regarde si le
        # conteneur peut la traiter

        # La touche entrée permet de passer au champ suivant
        if sequence.egale(BAS) or sequence.egale(SUITE):
            self.element_actif.gere_depart()
            if not self.suivant():
                self.minitel.bip()
            self.element_actif.gere_arrivee()
            return True

        # La combinaison Majuscule + entrée permet de passer au champ précédent
        if sequence.egale(HAUT) or sequence.egale(RETOUR):
            self.element_actif.gere_depart()
            if not self.precedent():
                self.minitel.bip()
            self.element_actif.gere_arrivee()
            return True

        return False
            
    def affiche(self):
        """Affichage du conteneur et de ses éléments

        À l’appel de cette méthode, le conteneur dessine le fond si la couleur
        de fond a été définie. Ensuite, elle demande à chacun des éléments
        contenus de se dessiner.

        Note:
            Les coordonnées du conteneur et les coordonnées des éléments sont
            indépendantes.

        """
        # Colorie le fond du conteneur si une couleur de fond a été définie
        if self.fond != None:
            for posy in range(self.posy, self.posy + self.hauteur):
                self.minitel.position(self.posx, posy)
                self.minitel.couleur(fond = self.fond)
                self.minitel.repeter(' ', self.largeur)

        # Demande à chaque élément de s’afficher
        for element in self.elements:
            element.affiche()

        # Si un élément actif a été défini, on lui donne la main
        if self.element_actif != None:
            self.element_actif.gere_arrivee()

    def ajoute(self, element):
        """Ajout d’un élément au conteneur

        Le conteneur maintient une liste ordonnées de ses éléments.

        Quand un élément est ajouté, si sa couleur n’a pas été définie, il
        prend celle du conteneur.

        Si aucun élément du conteneur n’est actif et que l’élément ajouté est
        activable, il devient automatiquement l’élément actif pour le
        conteneur.

        :param element:
            l’élément à ajouter à la liste ordonnée.
        
        :type element:
            un objet de classe UI ou de ses descendantes.
        """
        assert isinstance(element, UI)
        assert element not in self.elements

        # Attribue la couleur du conteneur à l’élément par défaut
        if element.couleur == None:
            element.couleur = self.couleur

        # Ajoute l’élément à la liste d’éléments du conteneur
        self.elements.append(element)

        if self.element_actif == None and element.activable == True:
            self.element_actif = element

    def suivant(self):
        """Passe à l’élément actif suivant

        Cette méthode sélectionne le prochain élément activable dans la liste
        à partir de l’élément actif.

        :returns:
            True si un élément actif suivant a été trouvé et sélectionné,
            False sinon.
        """
        # S’il n’y a pas d’éléments, il ne peut pas y avoir d’élément actif
        if len(self.elements) == 0:
            return False

        # Récupère l’index de l’élément actif
        if self.element_actif == None:
            index = -1
        else:
            index = self.elements.index(self.element_actif)

        # Recherche l’élément suivant qui soit activable
        while index < len(self.elements) - 1:
            index += 1
            if self.elements[index].activable == True:
                self.element_actif = self.elements[index]
                return True

        return False

    def precedent(self):
        """Passe à l’élément actif précédent

        Cette méthode sélectionne l’élément activable précédent dans la liste
        à partir de l’élément actif.

        :returns:
            True si un élément actif précédent a été trouvé et sélectionné,
            False sinon.
        """
        # S’il n’y a pas d’éléments, il ne peut pas y avoir d’élément actif
        if len(self.elements) == 0:
            return False

        # Récupère l’index de l’élément actif
        if self.element_actif == None:
            index = len(self.elements)
        else:
            index = self.elements.index(self.element_actif)

        # Recherche l’élément suivant qui soit activable
        while index > 0:
            index -= 1
            if self.elements[index].activable == True:
                self.element_actif = self.elements[index]
                return True

        return False

