#!/usr/bin/env python
# -*- coding: utf-8 -*-

from minitel.Minitel import Minitel
from minitel.ui.Menu import Menu

minitel = Minitel()

minitel.deviner_vitesse()
minitel.definir_vitesse(1200)
minitel.definir_mode('VIDEOTEX')
minitel.configurer_clavier(etendu = True, curseur = False, minuscule = True)
minitel.echo(False)
minitel.efface()
minitel.curseur(False)

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

menu = Menu(minitel, options, 5, 3, grille = False)
menu.affiche()

menu.executer()

minitel.close()

