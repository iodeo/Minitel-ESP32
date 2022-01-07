#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Sequence est un module permettant de gérer les séquences de caractères
pouvant être envoyées à un Minitel.

"""

# from unicodedata import normalize
from binascii import unhexlify

# Tables de conversion des caractères spéciaux
UNICODEVERSVIDEOTEX = {
    '£': '1923', '°': '1930', '±': '1931', 
    '←': '192C', '↑': '192D', '→': '192E', '↓': '192F', 
    '¼': '193C', '½': '193D', '¾': '193E', 
    'ç': '194B63', '’': '194B27', 
    'à': '194161', 'á': '194261', 'â': '194361', 'ä': '194861', 
    'è': '194165', 'é': '194265', 'ê': '194365', 'ë': '194865', 
    'ì': '194169', 'í': '194269', 'î': '194369', 'ï': '194869', 
    'ò': '19416F', 'ó': '19426F', 'ô': '19436F', 'ö': '19486F', 
    'ù': '194175', 'ú': '194275', 'û': '194375', 'ü': '194875', 
    'Œ': '196A', 'œ': '197A', 
    'ß': '197B', 'β': '197B'
}

UNICODEVERSAUTRE = {
    '£': '0E230F',
    '°': '0E5B0F', 'ç': '0E5C0F', '’': '27', '`': '60', '§': '0E5D0F',
    'à': '0E400F', 'è': '0E7F0F', 'é': '0E7B0F', 'ù': '0E7C0F'
}

class Sequence:
    """Une classe représentant une séquence de valeurs

    Une Séquence est une suite de valeurs prêtes à être envoyées à un Minitel.
    Ces valeurs respectent la norme ASCII.
    """
    def __init__(self, valeur = None, standard = 'VIDEOTEX'):
        """Constructeur de Sequence

        :param valeur:
            valeur à ajouter à la construction de l’objet. Si la valeur est à
            None, aucune valeur n’est ajoutée
        :type valeur:
            une chaîne de caractères, un entier, une liste, une séquence ou
            None

        :param standard:
            standard à utiliser pour la conversion unicode vers Minitel. Les
            valeurs possibles sont VIDEOTEX, MIXTE et TELEINFORMATIQUE (la
            casse est importante)
        :type standard:
            une chaîne de caractères
        """
        assert valeur == None or \
                isinstance(valeur, (list, int, str, Sequence))
        assert standard in ['VIDEOTEX', 'MIXTE', 'TELEINFORMATIQUE']

        self.valeurs = []
        self.longueur = 0
        self.standard = standard

        if valeur != None:
            self.ajoute(valeur)
        
    def ajoute(self, valeur):
        """Ajoute une valeur ou une séquence de valeurs

        La valeur soumise est d’abord canonisée par la méthode canonise avant
        d’être ajoutée à la séquence. Cela garantit que la séquence ne contient
        que des entiers représentant des caractères de la norme ASCII.

        :param valeur:
            valeur à ajouter
        :type valeur:
            une chaîne de caractères, un entier, une liste ou une Séquence
        """
        assert isinstance(valeur, (list, int, str, Sequence))

        self.valeurs += self.canonise(valeur)
        self.longueur = len(self.valeurs)

    def canonise(self, valeur):
        """Canonise une séquence de caractères

        Si une liste est soumise, quelle que soit sa profondeur, elle sera
        remise à plat. Une liste peut donc contenir des chaînes de caractères,
        des entiers ou des listes. Cette facilité permet la construction de
        séquences de caractères plus aisée. Cela facilite également la
        comparaison de deux séquences.

        :param valeur:
            valeur à canoniser
        :type valeur:
            une chaîne de caractères, un entier, une liste ou une Séquence

        :returns:
            Une liste de profondeur 1 d’entiers représentant des valeurs à la
            norme ASCII.

        Exemple::
            canonise(['dd', 32, ['dd', 32]]) retournera
            [100, 100, 32, 100, 100, 32]
        """
        assert isinstance(valeur, (list, int, str, Sequence))

        # Si la valeur est juste un entier, on le retient dans une liste
        if isinstance(valeur, int):
            return [valeur]

        # Si la valeur est une Séquence, ses valeurs ont déjà été canonisées
        if isinstance(valeur, Sequence):
            return valeur.valeurs

        # À ce point, le paramètre contient soit une chaîne de caractères, soit
        # une liste. L’une ou l’autre est parcourable par une boucle for ... in
        # Transforme récursivement chaque élément de la liste en entier
        canonise = []
        for element in valeur:
            if isinstance(element, str):
                # Cette boucle traite 2 cas : celui ou liste est une chaîne
                # unicode et celui ou element est une chaîne de caractères
                for caractere in element:
                    for ascii in self.unicode_vers_minitel(caractere):
                        canonise.append(ascii)
            elif isinstance(element, int):
                # Un entier a juste besoin d’être ajouté à la liste finale
                canonise.append(element)
            elif isinstance(element, list):
                # Si l’élément est une liste, on la canonise récursivement
                canonise = canonise + self.canonise(element)

        return canonise

    def unicode_vers_minitel(self, caractere):
        """Convertit un caractère unicode en son équivalent Minitel

        :param caractere:
            caractère à convertir
        :type valeur:
            une chaîne de caractères unicode

        :returns:
            une chaîne de caractères contenant une suite de caractères à
            destination du Minitel.
        """
        assert isinstance(caractere, str) and len(caractere) == 1

        if self.standard == 'VIDEOTEX':
            if caractere in UNICODEVERSVIDEOTEX:
                return unhexlify(UNICODEVERSVIDEOTEX[caractere])
        else:
            if caractere in UNICODEVERSAUTRE:
                return unhexlify(UNICODEVERSAUTRE[caractere])

#         return normalize('NFKD', caractere).encode('ascii', 'replace')
        return caractere.encode('ascii', 'replace')

    def egale(self, sequence):
        """Teste l’égalité de 2 séquences

        :param sequence:
            séquence à comparer. Si la séquence n’est pas un objet Sequence,
            elle est d’abord convertie en objet Sequence afin de canoniser ses
            valeurs.
        :type sequence:
            un objet Sequence, une liste, un entier, une chaîne de caractères
            ou une chaîne unicode

        :returns:
            True si les 2 séquences sont égales, False sinon
        """
        assert isinstance(sequence, (Sequence, list, int, str))

        # Si la séquence à comparer n’est pas de la classe Sequence, alors
        # on la convertit
        if not isinstance(sequence, Sequence):
            sequence = Sequence(sequence)

        return self.valeurs == sequence.valeurs

