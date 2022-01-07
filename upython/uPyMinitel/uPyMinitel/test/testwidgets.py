#!/usr/bin/env python
# -*- coding: utf-8 -*-

from minitel.Minitel import Minitel
from minitel.ui.ChampTexte import ChampTexte
from minitel.ui.Conteneur import Conteneur
from minitel.ui.Label import Label
from minitel.ui.Menu import Menu

# Initialisation du Minitel
minitel = Minitel()

minitel.deviner_vitesse()
minitel.identifier()
minitel.definir_vitesse(9600)
minitel.definir_mode('VIDEOTEX')
minitel.configurer_clavier(etendu = True, curseur = False, minuscule = True)
minitel.echo(False)
minitel.efface()
minitel.curseur(False)

# Création des widgets
conteneur = Conteneur(minitel, 1, 1, 32, 17, 'blanc', 'noir')

options = [
  'Nouveau',
  'Ouvrir',
  '-',
  'Enregistrer',
  'Enreg. sous...',
  'Rétablir',
  '-',
  'Aperçu',
  'Imprimer...',
  '-',
  'Fermer',
  'Quitter'
]

labelMenu = Label(minitel, 2, 2, "Menu", 'rouge')
menu = Menu(minitel, options, 9, 1)

labelNom = Label(minitel, 2, 15, "Nom", 'rouge')
champNom = ChampTexte(minitel, 10, 15, 20, 60)

labelPrenom = Label(minitel, 2, 16, "Prénom", 'rouge')
champPrenom = ChampTexte(minitel, 10, 16, 20, 60)

conteneur.ajoute(labelMenu)
conteneur.ajoute(menu)
conteneur.ajoute(labelNom)
conteneur.ajoute(champNom)
conteneur.ajoute(labelPrenom)
conteneur.ajoute(champPrenom)
conteneur.affiche()

conteneur.executer()

minitel.close()

