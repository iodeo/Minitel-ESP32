#!/usr/bin/env python
# -*- coding: utf-8 -*-

from minitel.Minitel import Minitel
from minitel.ui.ChampTexte import ChampTexte
from minitel.ui.Conteneur import Conteneur
from minitel.ui.Label import Label

minitel = Minitel()

minitel.deviner_vitesse()
minitel.identifier()
minitel.definir_vitesse(1200)
minitel.definir_mode('VIDEOTEX')
minitel.configurer_clavier(etendu = True, curseur = False, minuscule = True)
minitel.echo(False)
minitel.efface()
minitel.curseur(False)

conteneur = Conteneur(minitel, 1, 1, 40, 24)

labelNom = Label(minitel, 1, 10, "Nom")
champNom = ChampTexte(minitel, 16, 10, 20, 60)
labelPrenom = Label(minitel, 1, 12, "Prénom")
champPrenom = ChampTexte(minitel, 16, 12, 20, 60)
labelpass = Label(minitel, 1, 14, "Mot de passe")
champpass = ChampTexte(minitel, 16, 14, 20, 60, champ_cache=True)

conteneur.ajoute(labelNom)
conteneur.ajoute(champNom)
conteneur.ajoute(labelPrenom)
conteneur.ajoute(champPrenom)
conteneur.ajoute(labelpass)
conteneur.ajoute(champpass)
conteneur.affiche()

conteneur.executer()

minitel.close()

