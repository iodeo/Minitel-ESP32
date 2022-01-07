#!/usr/bin/env python
# -*- coding: utf-8 -*-

from minitel.Minitel import Minitel
from minitel.ui.ChampTexte import ChampTexte

minitel = Minitel()
minitel.deviner_vitesse()
minitel.definir_vitesse(1200)
minitel.definir_mode('VIDEOTEX')
#minitel.configurer_clavier(etendu = True, curseur = False, minuscule = True)
minitel.echo(False)
minitel.efface()
minitel.curseur(False)

champ = ChampTexte(minitel, 10, 10, 20, 60, 'Hello world')
champ.affiche()
champ.gere_arrivee()
champ.executer()
minitel.close()

