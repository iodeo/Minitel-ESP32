#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Minitel est un module permettant de piloter un Minitel depuis un script
écrit en Python.
"""

from machine import UART       # Liaison physique avec le Minitel
from time import ticks_ms

from minitel.Sequence import Sequence # Gestion des séquences de caractères

from minitel.constantes import (SS2, SEP, ESC, CSI, PRO1, PRO2, PRO3, MIXTE1,
    MIXTE2, TELINFO, ENQROM, SOH, EOT, TYPE_MINITELS, STATUS_FONCTIONNEMENT,
    LONGUEUR_PRO2, STATUS_TERMINAL, REP_STATUS_TERMINAL, REP_STATUS_VITESSE,
    PROG, START, STOP, LONGUEUR_PRO3, RCPT_CLAVIER, ETEN, C0, MINUSCULES, RS,
    US, VT, LF, BS, TAB, CON, COF, AIGUILLAGE_ON, AIGUILLAGE_OFF, RCPT_MODEM,
    EMET_CLAVIER, FF, CAN, BEL, CR, SO, SI, B300, B1200, B4800, B9600, REP,
    COULEURS_MINITEL, CAPACITES_BASIQUES, CONSTRUCTEURS)

def normaliser_couleur(couleur):
    """Retourne le numéro de couleur du Minitel.

    À partir d’une couleur fournie sous la forme d’une chaîne avec le
    nom de la couleur en français ou un entier indiquant un niveau de
    gris, cette fonction retourne le numéro de la couleur correspondante
    pour le Minitel.

    :param couleur:
        Les valeurs acceptées sont noir, rouge, vert, jaune, bleu,
        magenta, cyan, blanc, et les entiers de 0 (noir) à 7 (blanc)
    :type couleur:
        une chaîne de caractères ou un entier

    :returns:
        Le numéro de la couleur correspondante sur le Minitel ou None si
        la couleur demandée n’est pas valide.
    """
    assert isinstance(couleur, (str, int))

    # On convertit la couleur en chaîne de caractères pour que l’appelant
    # puisse utiliser indifféremment '0' (str) ou 0 (int).
    couleur = str(couleur)

    if couleur in COULEURS_MINITEL:
        return COULEURS_MINITEL[couleur]

    return None

class Minitel:
    """Une classe de pilotage du Minitel via un port série

    Présentation
    ============

    La classe Minitel permet d’envoyer et de recevoir des séquences de
    caractères vers et depuis un Minitel dans un programme écrit en Python.
    Elle fonctionne via une liaison série entre l’ordinateur et le Minitel.

    Par défaut, elle utilise le port UART 2 comme périphérique. En effet, l’une
    des manières les plus simples de relier un Minitel à un microcontrôleur
    consiste à utiliser un port  UART avec une interface à collecteur ouvert.
    La péri-informatique du Minitel fonctionne en TTL (0v/5v) et non en RS232
    (-12v/12v).

    Tant que le périphérique sélectionné est un périphérique série, cette
    classe ne devrait pas poser de problème pour communiquer avec le Minitel.
    Par exemple, il est tout à fait possible de créer un proxy série en
    utilisant un Arduino relié en USB à l’ordinateur et dont certaines
    broches seraient relié au Minitel.

    La classe Minitel permet de déterminer la vitesse de fonctionnement du
    Minitel, d’identifier le modèle, de le configurer et d’envoyer et recevoir
    des séquences de caractères.

    Compte tenu de son fonctionnement en threads, le programme principal
    utilisant cette classe n’a pas à se soucier d’être disponible pour recevoir
    les séquences de caractères envoyées par le Minitel.

    Démarrage rapide
    ================

    Le cycle de vie d’un objet Minitel consiste en la création, la
    détermination de la vitesse du Minitel, de ses capacités, l’utilisation
    du Minitel par l’application et la libération des ressources::
        
        from minitel.Minitel import Minitel

        minitel = Minitel()

        minitel.deviner_vitesse()
        minitel.identifier()

        # ...
        # Utilisation de l’objet minitel
        # ...

        minitel.close()

    """
    def __init__(self, uart_num = 2):
        """Constructeur de Minitel

        La connexion série est établie selon le standard de base du Minitel.
        À l’allumage le Minitel est configuré à 1200 bps, 7 bits, parité paire,
        mode Vidéotex.

        Cela peut ne pas correspondre à la configuration réelle du Minitel au
        moment de l’exécution. Cela n’est toutefois pas un problème car la
        connexion série peut être reconfigurée à tout moment.

        :param uart_num:
            Le port uart utilisé. Par défaut, le port 2 de l'ESP32 est utilisé
        :type uart_num:
            Integer
    
        """
        assert isinstance(uart_num, int)
        self.uart_num = uart_num

        # Initialise l’état du Minitel
        self.mode = 'VIDEOTEX'
        self.vitesse = 1200

        # Initialise la liste des capacités du Minitel
        self.capacite = CAPACITES_BASIQUES

        # Initialise la connexion avec le Minitel
        self._minitel = UART(
            self.uart_num,
            baudrate = 1200,  # vitesse à 1200 bps, le standard Minitel
            bits = 7,         # taille de caractère à 7 bits
            parity = 0,       # parité paire
            stop = 1,         # 1 bit d’arrêt
            timeout = 1,      # 1s de timeout
            timeout_char = 1, # 1s de timeout entre caracteres
            flow   = 0        # pas de contrôle matériel
        )

    def close(self):
        """Ferme la connexion avec le Minitel

        """
        self._minitel.deinit()

    def envoyer_brut(self, byte):
        self._minitel.write(byte)

    def envoyer(self, contenu):
        """Envoi de séquence de caractères 

        Envoie une séquence de caractère en direction du Minitel.

        :param contenu:
            Une séquence de caractères interprétable par la classe Sequence.
        :type contenu:
            un objet Sequence, une chaîne de caractères ou unicode, une liste,
            un entier
        """
        # Convertit toute entrée en objet Sequence
        if not isinstance(contenu, Sequence):
            contenu = Sequence(contenu)

        # Envois les caractères un par un
        for valeur in contenu.valeurs:
            self.envoyer_brut(chr(valeur).encode())

    def recevoir(self, bloque = False, attente = None, nbytes = 1):
        """Lit un caractère en provenance du Minitel

        Retourne un caractère reçu du Minitel

        :param bloque:
            True pour attendre un caractère s’il n’y en a pas
            False pour ne pas attendre etn retourner immédiatement.
        :type bloque:
            un booléen

        :param attente:
            attente en secondes, valeurs en dessous de la seconde
            acceptées. Valide uniquement en mode bloque = True
            Si attente = None et bloque = True, alors on attend
            indéfiniment qu'un caractère arrive. 
        :type attente:
            un entier, ou None
        
        :param nbytes:
            nombre de byte à lire au maximum
        """
        assert bloque in [True, False]
        assert isinstance(attente, (int,float)) or attente == None
        assert isinstance(nbytes, int)

        # Convertit les secondes en millisecondes
        if attente:
            attente = attente * 1000

        caractere = ''

        if bloque:
            start_ms = ticks_ms()
            while not self._minitel.any():
                if attente:
                    if ticks_ms()-start_ms > attente:
                        break
                pass

        if self._minitel.any():
            caractere = self._minitel.read(nbytes).decode()

        return caractere

    def recevoir_sequence(self,bloque = True, attente=None):
        """Lit une séquence en provenance du Minitel

        Retourne un objet Sequence reçu depuis le Minitel. Cette fonction
        analyse les envois du Minitel pour en faire une séquence consistante
        du point de vue du Minitel. Par exemple, si le Minitel envoie un
        caractère SS2, SEP ou ESC, celui-ci ne fait qu’annoncer une suite de
        caractères désignant un résultat ou un caractère non existant dans la
        norme ASCII. Par contre, le nombre de caractères pouvant être reçus
        après des caractères spéciaux est normalisé. Cela permet de savoir
        exactement le nombre de caractères qui vont constituer la séquence.

        C’est cette méthode qui doit être utilisée plutôt que la méthode
        recevoir lorsqu’on dialogue avec le Minitel.

        :param bloque:
            True pour attendre une séquence s’il n’y en a pas dans la
            file d’attente de réception. False pour ne pas attendre et
            retourner immédiatement.
        :type bloque:
            un booléen

        :param attente:
            attente en secondes, valeurs en dessous de la seconde
            acceptées. Valide uniquement en mode bloque = True
            Si attente = None et bloque = True, alors on attend
            indéfiniment qu'un caractère arrive. 
        :type attente:
            un entier, ou None

        :returns:
            un objet Sequence
        """
        # Crée une séquence
        sequence = Sequence()

        # Ajoute le premier caractère lu à la séquence en mode bloquant
        sequence.ajoute(self.recevoir(bloque = bloque, attente = attente))
        assert sequence.longueur != 0

        # Teste le caractère reçu
        if sequence.valeurs[-1] in [SS2, SEP]:
            # Une séquence commençant par SS2 ou SEP aura une longueur de 2
            sequence.ajoute(self.recevoir(bloque = True))
        elif sequence.valeurs[-1] == ESC:
            # Les séquences ESC ont des tailles variables allant de 1 à 4
            # Essaie de lire un caractère avec un temps d’attente de 1/10s
            # Cela permet de lire la touche la touche Esc qui envoie
            # uniquement le code ESC sans rien après.
            caractere = self.recevoir(bloque = True, attente = 0.1)
            if not caractere:
                return sequence
            sequence.ajoute(caractere)
            # Une séquence CSI commence par ESC, 0x5b
            if sequence.valeurs == CSI:
                # Une séquence CSI appelle au moins 1 caractère
                sequence.ajoute(self.recevoir(bloque = True))
                if sequence.valeurs[-1] in [0x32, 0x34]:
                    # La séquence ESC, 0x5b, 0x32/0x34 appelle un dernier
                    # caractère
                    sequence.ajoute(self.recevoir(bloque = True))

        return sequence

    def appeler(self, contenu, attente):
        """Envoie une séquence au Minitel et attend sa réponse.

        Cette méthode permet d’envoyer une commande au Minitel (configuration,
        interrogation d’état) et d’attendre sa réponse. Cette fonction attend
        au maximum 1 seconde avant d’abandonner. Dans ce cas, une séquence
        vide est retournée.

        :param contenu:
            Une séquence de caractères interprétable par la classe
            Sequence
        :type contenu:
            un objet Sequence, une chaîne de caractères, une chaîne unicode
            ou un entier

        :param attente:
            Nombre de caractères attendu de la part du Minitel en
            réponse à notre envoi.
        :type attente:
            un entier

        :returns:
            un objet Sequence contenant la réponse du Minitel à la commande
            envoyée.
        """
        assert isinstance(attente, int)

        # Envoie la séquence
        self.envoyer(contenu)

        # Tente de recevoir le nombre de caractères indiqué par le paramètre
        # attente avec un délai d’1 seconde.
        retour = Sequence()
        for _ in range(0, attente):
            # Attend un caractère
            entree_bytes = self.recevoir(bloque = True, attente = 1)
            if entree_bytes:
                retour.ajoute(entree_bytes)

        return retour

    def definir_mode(self, mode = 'VIDEOTEX'):
        """Définit le mode de fonctionnement du Minitel.

        Le Minitel peut fonctionner selon 3 modes : VideoTex (le mode standard
        du Minitel, celui lors de l’allumage), Mixte ou TéléInformatique (un
        mode 80 colonnes).

        La méthode definir_mode prend en compte le mode courant du Minitel pour
        émettre la bonne commande.

        :param mode:
            une valeur parmi les suivantes : VIDEOTEX,
            MIXTE ou TELEINFORMATIQUE (la casse est importante).
        :type mode:
            une chaîne de caractères

        :returns:
            False si le changement de mode n’a pu avoir lieu, True sinon.
        """
        assert isinstance(mode, str)

        # 3 modes sont possibles
        if mode not in ['VIDEOTEX', 'MIXTE', 'TELEINFORMATIQUE']:
            return False

        # Si le mode demandé est déjà actif, ne fait rien
        if self.mode == mode:
            return True

        resultat = False

        # Il y a 9 cas possibles, mais seulement 6 sont pertinents. Les cas
        # demandant de passer de VIDEOTEX à VIDEOTEX, par exemple, ne donnent
        # lieu à aucune transaction avec le Minitel
        if self.mode == 'TELEINFORMATIQUE' and mode == 'VIDEOTEX':
            retour = self.appeler([CSI, 0x3f, 0x7b], 2)
            resultat = retour.egale([SEP, 0x5e])
        elif self.mode == 'TELEINFORMATIQUE' and mode == 'MIXTE':
            # Il n’existe pas de commande permettant de passer directement du
            # mode TéléInformatique au mode Mixte. On effectue donc la
            # transition en deux étapes en passant par le mode Videotex
            retour = self.appeler([CSI, 0x3f, 0x7b], 2)
            resultat = retour.egale([SEP, 0x5e])

            if not resultat:
                return False

            retour = self.appeler([PRO2, MIXTE1], 2)
            resultat = retour.egale([SEP, 0x70])
        elif self.mode == 'VIDEOTEX' and mode == 'MIXTE':
            retour = self.appeler([PRO2, MIXTE1], 2)
            resultat = retour.egale([SEP, 0x70])
        elif self.mode == 'VIDEOTEX' and mode == 'TELEINFORMATIQUE':
            retour = self.appeler([PRO2, TELINFO], 4)
            resultat = retour.egale([CSI, 0x3f, 0x7a])
        elif self.mode == 'MIXTE' and mode == 'VIDEOTEX':
            retour = self.appeler([PRO2, MIXTE2], 2)
            resultat = retour.egale([SEP, 0x71])
        elif self.mode == 'MIXTE' and mode == 'TELEINFORMATIQUE':
            retour = self.appeler([PRO2, TELINFO], 4)
            resultat = retour.egale([CSI, 0x3f, 0x7a])

        # Si le changement a eu lieu, on garde le nouveau mode en mémoire
        if resultat:
            self.mode = mode

        return resultat

    def identifier(self):
        """Identifie le Minitel connecté.

        Cette méthode doit être appelée une fois la connexion établie avec le
        Minitel afin de déterminer les fonctionnalités et caractéristiques
        disponibles.

        Aucune valeur n’est retournée. À la place, l’attribut capacite de
        l’objet contient un dictionnaire de valeurs renseignant sur les
        capacités du Minitel :

        - capacite['nom'] -- Nom du Minitel (ex. Minitel 2)
        - capacite['retournable'] -- Le Minitel peut-il être retourné et
          servir de modem ? (True ou False)
        - capacite['clavier'] -- Clavier (None, ABCD ou Azerty)
        - capacite['vitesse'] -- Vitesse maxi en bps (1200, 4800 ou 9600)
        - capacite['constructeur'] -- Nom du constructeur (ex. Philips)
        - capacite['80colonnes'] -- Le Minitel peut-il afficher 80
          colonnes ? (True ou False)
        - capacite['caracteres'] -- Peut-on redéfinir des caractères ?
          (True ou False)
        - capacite['version'] -- Version du logiciel (une lettre)
        """
        self.capacite = CAPACITES_BASIQUES

        # Émet la commande d’identification
        retour = self.appeler([PRO1, ENQROM], 5)

        # Teste la validité de la réponse
        if (retour.longueur != 5 or
            retour.valeurs[0] != SOH or
            retour.valeurs[4] != EOT):
            return

        # Extrait les caractères d’identification
        constructeur_minitel = chr(retour.valeurs[1])
        type_minitel         = chr(retour.valeurs[2])
        version_logiciel     = chr(retour.valeurs[3])

        # Types de Minitel
        if type_minitel in TYPE_MINITELS:
            self.capacite = TYPE_MINITELS[type_minitel]

        if constructeur_minitel in CONSTRUCTEURS:
            self.capacite['constructeur'] = CONSTRUCTEURS[constructeur_minitel]

        self.capacite['version'] = version_logiciel

        # Correction du constructeur
        if constructeur_minitel == 'B' and type_minitel == 'v':
            self.capacite['constructeur'] = 'Philips'
        elif constructeur_minitel == 'C':
            if version_logiciel == ['4', '5', ';', '<']:
                self.capacite['constructeur'] = 'Telic ou Matra'

        # Détermine le mode écran dans lequel se trouve le Minitel
        retour = self.appeler([PRO1, STATUS_FONCTIONNEMENT], LONGUEUR_PRO2)

        if retour.longueur != LONGUEUR_PRO2:
            # Le Minitel est en mode Téléinformatique car il ne répond pas
            # à une commande protocole
            self.mode = 'TELEINFORMATIQUE'
        elif retour.valeurs[3] & 1 == 1:
            # Le bit 1 du status fonctionnement indique le mode 80 colonnes
            self.mode = 'MIXTE'
        else:
            # Par défaut, on considère qu’on est en mode Vidéotex
            self.mode = 'VIDEOTEX'

    def deviner_vitesse(self):
        """Deviner la vitesse de connexion avec le Minitel.

        Cette méthode doit être appelée juste après la création de l’objet
        afin de déterminer automatiquement la vitesse de transmission sur
        laquelle le Minitel est réglé.

        Pour effectuer la détection, la méthode deviner_vitesse va tester les
        vitesses 9600 bps, 4800 bps, 1200 bps et 300 bps (dans cet ordre) et
        envoyer à chaque fois une commande PRO1 de demande de statut terminal.
        Si le Minitel répond par un acquittement PRO2, on a détecté la vitesse.

        En cas de détection, la vitesse est enregistré dans l’attribut vitesse
        de l’objet.

        :returns:
            La méthode retourne la vitesse en bits par seconde ou -1 si elle
            n’a pas pu être déterminée.
        """
        # Vitesses possibles jusqu’au Minitel 2
        vitesses = [9600, 4800, 1200, 300]

        for vitesse in vitesses:
            # Configure le port série à la vitesse à tester
            self._minitel.deinit()
            self._minitel = UART(
                self.uart_num,
                baudrate = vitesse, # vitesse à 1200 bps, le standard Minitel
                bits = 7,           # taille de caractère à 7 bits
                parity = 0,         # parité paire
                stop = 1,           # 1 bit d’arrêt
                timeout = 1,        # 1s de timeout
                timeout_char = 1,   # 1s de timeout entre caracteres
                flow   = 0          # pas de contrôle matériel
            )


            # Envoie une demande de statut terminal
            retour = self.appeler([PRO1, STATUS_TERMINAL], LONGUEUR_PRO2)

            # Le Minitel doit renvoyer un acquittement PRO2
            if retour.longueur == LONGUEUR_PRO2:
                if retour.valeurs[2] == REP_STATUS_TERMINAL:
                    self.vitesse = vitesse
                    return vitesse

        # La vitesse n’a pas été trouvée
        return -1
    
    def recuperation(self):
        """Tente de réactiver le langage protocole en ramenant au mode VIDEOTEX

        Cette méthode doit être appelée si la méthode deviner_vitesse() n'a pas
        abouti, supposant que le minitel est en mode téléinformatique.

        Pour effectuer la récupération, la méthode va tester les vitesses
        9600 bps, 4800 bps, 1200 bps et 300 bps (dans cet ordre) et
        envoyer une demande de passage en mode VIDEOTEX.
        Si la méthode renvoie True, on a ramené le minitel en mode Videotex
        et trouvé la vitesse

        En cas de détection, la vitesse est enregistré dans l’attribut vitesse
        de l’objet, et le mode VIDEOTEX dans l'attribut mode

        :returns:
            La méthode retourne la vitesse en bits par seconde ou -1 si elle
            n’a pas pu être déterminée.
        """
        # Vitesses possibles jusqu’au Minitel 2
        vitesses = [9600, 4800, 1200, 300]
        
        # On suppose que le Minitel est en mode Téléinformatique
        self.mode = 'TELEINFORMATIQUE'

        for vitesse in vitesses:
            # Configure le port série à la vitesse à tester
            self._minitel.deinit()
            self._minitel = UART(
                self.uart_num,
                baudrate = vitesse, # vitesse à 1200 bps, le standard Minitel
                bits = 7,           # taille de caractère à 7 bits
                parity = 0,         # parité paire
                stop = 1,           # 1 bit d’arrêt
                timeout = 1,        # 1s de timeout
                timeout_char = 1,   # 1s de timeout entre caracteres
                flow   = 0          # pas de contrôle matériel
            )


            # Envoie la demande de passage en mode VIDEOTEX
            retour = self.appeler([CSI, 0x3f, 0x7b], 2)
            resultat = retour.egale([SEP, 0x5e])

            # La méthode doit renvoyer True
            if resultat:
                self.mode = 'VIDEOTEX'
                self.vitesse = vitesse
                return vitesse

        # La vitesse n’a pas été trouvée
        return -1
    

    def definir_vitesse(self, vitesse):
        """Programme le Minitel et le port série pour une vitesse donnée.

        Pour changer la vitesse de communication entre l’ordinateur et le
        Minitel, le développeur doit d’abord s’assurer que la connexion avec
        le Minitel a été établie à la bonne vitesse (voir la méthode
        deviner_vitesse).

        Cette méthode ne doit être appelée qu’après que le Minitel ait été
        identifié (voir la méthode identifier) car elle se base sur les
        capacités détectées du Minitel.

        La méthode envoie d’abord une commande de réglage de vitesse au Minitel
        et, si celui-ci l’accepte, configure le port série à la nouvelle
        vitesse.

        :param vitesse:
            vitesse en bits par seconde. Les valeurs acceptées sont 300, 1200,
            4800 et 9600. La valeur 9600 n’est autorisée qu’à partir du Minitel
            2
        :type vitesse:
            un entier

        :returns:
            True si la vitesse a pu être programmée, False sinon.
        """
        assert isinstance(vitesse, int)
        # Si la vitesse est deja la bonne, on ne fait rien
        if vitesse == self.vitesse:
            return True
        # Vitesses possibles jusqu’au Minitel 2
        vitesses = {300: B300, 1200: B1200, 4800: B4800, 9600: B9600}

        # Teste la validité de la vitesse demandée
#         if vitesse not in vitesses or vitesse > self.capacite['vitesse']:
#             return False

        # Envoie une commande protocole de programmation de vitesse
        retour = self.appeler([PRO2, PROG, vitesses[vitesse]], LONGUEUR_PRO2)

        # Le Minitel doit renvoyer un acquittement PRO2+REP_STATUS_VITESSE
        if retour.longueur == LONGUEUR_PRO2:
            if retour.valeurs[2] == REP_STATUS_VITESSE:
                # Si on peut lire un acquittement PRO2 avant d’avoir régler la
                # vitesse du port série, c’est que le Minitel ne peut pas utiliser
                # la vitesse demandée
                return False

        # Configure le port série à la nouvelle vitesse
        self._minitel.deinit()
        self._minitel = UART(
            self.uart_num,
            baudrate = vitesse, # vitesse à 1200 bps, le standard Minitel
            bits = 7,           # taille de caractère à 7 bits
            parity = 0,         # parité paire
            stop = 1,           # 1 bit d’arrêt
            timeout = 1,        # 1s de timeout
            timeout_char = 1,   # 1s de timeout entre caracteres
            flow   = 0          # pas de contrôle matériel
        )
        self.vitesse = vitesse

        return True

    def configurer_clavier(self, etendu = False, curseur = False,
                           minuscule = False):
        """Configure le fonctionnement du clavier.

        Configure le fonctionnement du clavier du Minitel. Cela impacte les
        codes et caractères que le Minitel peut envoyer à l’ordinateur en
        fonction des touches appuyées (touches alphabétiques, touches de
        fonction, combinaisons de touches etc.).

        La méthode renvoie True si toutes les commandes de configuration ont
        correctement été traitées par le Minitel. Dès qu’une commande échoue,
        la méthode arrête immédiatement et retourne False.

        :param etendu:
            True pour un clavier en mode étendu, False pour un clavier en mode
            normal
        :type etendu:
            un booléen

        :param curseur:
            True si les touches du curseur doivent être gérées, False sinon
        :type curseur:
            un booléen

        :param minuscule:
            True si l’appui sur une touche alphabétique sans appui simultané
            sur la touche Maj/Shift doit générer une minuscule, False s’il
            doit générer une majuscule.
        :type minuscule:
            un booléen
        """
        assert etendu in [True, False]
        assert curseur in [True, False]
        assert minuscule in [True, False]

        # Les commandes clavier fonctionnent sur un principe de bascule
        # start/stop
        bascules = { True: START, False: STOP }

        # Crée les séquences des 3 appels en fonction des arguments
        appels = [
            ([PRO3, bascules[etendu   ], RCPT_CLAVIER, ETEN], LONGUEUR_PRO3),
            ([PRO3, bascules[curseur  ], RCPT_CLAVIER, C0  ], LONGUEUR_PRO3),
            ([PRO2, bascules[minuscule], MINUSCULES        ], LONGUEUR_PRO2)
        ]

        # Envoie les commandes une par une
        for appel in appels:
            commande = appel[0] # Premier élément du tuple = commande
            longueur = appel[1] # Second élément du tuple = longueur réponse

            retour = self.appeler(commande, longueur)

            if retour.longueur != longueur:
                return False

        return True

    def couleur(self, caractere = None, fond = None):
        """Définit les couleurs utilisées pour les prochains caractères.

        Les couleurs possibles sont noir, rouge, vert, jaune, bleu, magenta,
        cyan, blanc et un niveau de gris de 0 à 7.        

        Note:
        En Videotex, la couleur de fond ne s’applique qu’aux délimiteurs. Ces
        délimiteurs sont l’espace et les caractères semi-graphiques. Définir
        la couleur de fond et afficher immédiatement après un caractère autre
        qu’un délimiteur (une lettre par exemple) n’aura aucun effet.

        Si une couleur est positionnée à None, la méthode n’émet aucune
        commande en direction du Minitel.

        Si une couleur n’est pas valide, elle est simplement ignorée.

        :param caractere:
            couleur à affecter à l’avant-plan.
        :type caractere:
            une chaîne de caractères, un entier ou None

        :param fond:
            couleur à affecter à l’arrière-plan.
        :type fond:
            une chaîne de caractères, un entier ou None
        """
        assert isinstance(caractere, (str, int)) or caractere == None
        assert isinstance(fond, (str, int)) or fond == None

        # Définit la couleur d’avant-plan (la couleur du caractère)
        if caractere != None:
            couleur = normaliser_couleur(caractere)
            if couleur != None:
                self.envoyer([ESC, 0x40 + couleur])

        # Définit la couleur d’arrière-plan (la couleur de fond)
        if fond != None:
            couleur = normaliser_couleur(fond)
            if couleur != None:
                self.envoyer([ESC, 0x50 + couleur])

    def position(self, colonne, ligne, relatif = False):
        """Définit la position du curseur du Minitel

        Note:
        Cette méthode optimise le déplacement du curseur, il est donc important
        de se poser la question sur le mode de positionnement (relatif vs
        absolu) car le nombre de caractères générés peut aller de 1 à 5.

        Sur le Minitel, la première colonne a la valeur 1. La première ligne
        a également la valeur 1 bien que la ligne 0 existe. Cette dernière
        correspond à la ligne d’état et possède un fonctionnement différent
        des autres lignes.

        :param colonne:
            colonne à laquelle positionner le curseur
        :type colonne:
            un entier relatif

        :param ligne:
            ligne à laquelle positionner le curseur
        :type ligne:
            un entier relatif

        :param relatif:
            indique si les coordonnées fournies sont relatives
            (True) par rapport à la position actuelle du curseur ou si
            elles sont absolues (False, valeur par défaut)
        :type relatif:
            un booléen
        """
        assert isinstance(colonne, int)
        assert isinstance(ligne, int)
        assert relatif in [True, False]

        if not relatif:
            # Déplacement absolu
            if colonne == 1 and ligne == 1:
                self.envoyer([RS])
            else:
                self.envoyer([US, 0x40 + ligne, 0x40 + colonne])
        else:
            # Déplacement relatif par rapport à la position actuelle
            if ligne != 0:
                if ligne >= -4 and ligne <= -1:
                    # Déplacement court en haut
                    self.envoyer([VT]*-ligne)
                elif ligne >= 1 and ligne <= 4:
                    # Déplacement court en bas
                    self.envoyer([LF]*ligne)
                else:
                    # Déplacement long en haut ou en bas
                    direction = { True: 'B', False: 'A'}
                    self.envoyer([CSI, str(ligne), direction[ligne < 0]])

            if colonne != 0:
                if colonne >= -4 and colonne <= -1:
                    # Déplacement court à gauche
                    self.envoyer([BS]*-colonne)
                elif colonne >= 1 and colonne <= 4:
                    # Déplacement court à droite
                    self.envoyer([TAB]*colonne)
                else:
                    # Déplacement long à gauche ou à droite
                    direction = { True: 'C', False: 'D'}
                    self.envoyer([CSI, str(colonne), direction[colonne < 0]])

    def taille(self, largeur = 1, hauteur = 1):
        """Définit la taille des prochains caractères

        Le Minitel est capable d’agrandir les caractères. Quatres tailles sont
        disponibles :

        - largeur = 1, hauteur = 1: taille normale
        - largeur = 2, hauteur = 1: caractères deux fois plus larges
        - largeur = 1, hauteur = 2: caractères deux fois plus hauts
        - largeur = 2, hauteur = 2: caractères deux fois plus hauts et larges

        Note:
        Cette commande ne fonctionne qu’en mode Videotex.

        Le positionnement avec des caractères deux fois plus hauts se fait par
        rapport au bas du caractère.

        :param largeur:
            coefficiant multiplicateur de largeur (1 ou 2)
        :type largeur:
            un entier

        :param hauteur:
            coefficient multiplicateur de hauteur (1 ou 2)
        :type hauteur:
            un entier
        """
        assert largeur in [1, 2]
        assert hauteur in [1, 2]

        self.envoyer([ESC, 0x4c + (hauteur - 1) + (largeur - 1) * 2])

    def effet(self, soulignement = None, clignotement = None, inversion = None):
        """Active ou désactive des effets

        Le Minitel dispose de 3 effets sur les caractères : soulignement,
        clignotement et inversion vidéo.

        :param soulignement:
            indique s’il faut activer le soulignement (True) ou le désactiver
            (False)
        :type soulignement:
            un booléen ou None

        :param clignotement:
            indique s’il faut activer le clignotement (True) ou le désactiver
            (False)
        :type clignotement:
            un booléen ou None

        :param inversion:
            indique s’il faut activer l’inverson vidéo (True) ou la désactiver
            (False)
        :type inversion:
            un booléen ou None
        """
        assert soulignement in [True, False, None]
        assert clignotement in [True, False, None]
        assert inversion in [True, False, None]

        # Gère le soulignement
        soulignements = {True: [ESC, 0x5a], False: [ESC, 0x59], None: None}
        self.envoyer(soulignements[soulignement])

        # Gère le clignotement
        clignotements = {True: [ESC, 0x48], False: [ESC, 0x49], None: None}
        self.envoyer(clignotements[clignotement])

        # Gère l’inversion vidéo
        inversions = {True: [ESC, 0x5d], False: [ESC, 0x5c], None: None}
        self.envoyer(inversions[inversion])

    def curseur(self, visible):
        """Active ou désactive l’affichage du curseur

        Le Minitel peut afficher un curseur clignotant à la position
        d’affichage des prochains caractères.

        Il est intéressant de le désactiver quand l’ordinateur doit envoyer
        de longues séquences de caractères car le Minitel va chercher à
        afficher le curseur pour chaque caractère affiché, générant un effet
        peu agréable.

        :param visible:
            indique s’il faut activer le curseur (True) ou le rendre invisible
            (False)
        :type visible:
            un booléen
        """
        assert visible in [True, False]

        etats = {True: CON, False: COF}
        self.envoyer([etats[visible]])

    def echo(self, actif):
        """Active ou désactive l’écho clavier

        Par défaut, le Minitel envoie tout caractère tapé au clavier à la fois
        à l’écran et sur la prise péri-informatique. Cette astuce évite à
        l’ordinateur de dévoir renvoyer à l’écran le dernière caractère tapé,
        économisant ainsi de la bande passante.

        Dans le cas où l’ordinateur propose une interface utilisateur plus
        poussée, il est important de pouvoir contrôler exactement ce qui est
        affiché par le Minitel.

        La méthode retourne True si la commande a bien été traitée par le
        Minitel, False sinon.

        :param actif:
            indique s’il faut activer l’écho (True) ou le désactiver (False)
        :type actif:
            un booléen

        :returns:
            True si la commande a été acceptée par le Minitel, False sinon.
        """
        assert actif in [True, False]

        actifs = {
            True: [PRO3, AIGUILLAGE_ON, RCPT_MODEM, EMET_CLAVIER],
            False: [PRO3, AIGUILLAGE_OFF, RCPT_MODEM, EMET_CLAVIER]
        }
        retour = self.appeler(actifs[actif], LONGUEUR_PRO3)
        
        return retour.longueur == LONGUEUR_PRO3

    def efface(self, portee = 'tout'):
        """Efface tout ou partie de l’écran

        Cette méthode permet d’effacer :


        :param portee:
            indique la portée de l’effacement :

            - tout l’écran ('tout'),
            - du curseur jusqu’à la fin de la ligne ('finligne'),
            - du curseur jusqu’au bas de l’écran ('finecran'),
            - du début de l’écran jusqu’au curseur ('debutecran'),
            - du début de la ligne jusqu’au curseur ('debut_ligne'),
            - la ligne entière ('ligne'),
            - la ligne de statut, rangée 00 ('statut'),
            - tout l’écran et la ligne de statut ('vraimenttout').
        :type porte:
            une chaîne de caractères
        """
        portees = {
            'tout': [FF],
            'finligne': [CAN],
            'finecran': [CSI, 0x4a],
            'debutecran': [CSI, 0x31, 0x4a],
            #'tout': [CSI, 0x32, 0x4a],
            'debut_ligne': [CSI, 0x31, 0x4b],
            'ligne': [CSI, 0x32, 0x4b],
            'statut': [US, 0x40, 0x41, CAN, LF],
            'vraimenttout': [FF, US, 0x40, 0x41, CAN, LF]
        }

        assert portee in portees

        self.envoyer(portees[portee])

    def repeter(self, caractere, longueur):
        """Répéter un caractère

        :param caractere:
            caractère à répéter
        :type caractere:
            une chaîne de caractères

        :param longueur:
            le nombre de fois où le caractère est répété
        :type longueur:
            un entier positif
        """
        assert isinstance(longueur, int)
        assert longueur > 0 and longueur <= 40
        assert isinstance(caractere, (str, int, list))
        assert isinstance(caractere, int) or len(caractere) == 1

        self.envoyer([caractere, REP, 0x40 + longueur - 1])

    def bip(self):
        """Émet un bip

        Demande au Minitel d’émettre un bip
        """
        self.envoyer([BEL])

    def debut_ligne(self):
        """Retour en début de ligne

        Positionne le curseur au début de la ligne courante.
        """
        self.envoyer([CR])

    def supprime(self, nb_colonne = None, nb_ligne = None):
        """Supprime des caractères après le curseur

        En spécifiant un nombre de colonnes, cette méthode supprime des
        caractères après le curseur, le Minitel ramène les derniers caractères
        contenus sur la ligne.
        
        En spécifiant un nombre de lignes, cette méthode supprime des lignes
        sous la ligne contenant le curseur, remontant les lignes suivantes.

        :param nb_colonne:
            nombre de caractères à supprimer
        :type nb_colonne:
            un entier positif
        :param nb_ligne:
            nombre de lignes à supprimer
        :type nb_ligne:
            un entier positif
        """
        assert (isinstance(nb_colonne, int) and nb_colonne >= 0) or \
                nb_colonne == None
        assert (isinstance(nb_ligne, int) and nb_ligne >= 0) or \
                nb_ligne == None

        if nb_colonne != None:
            self.envoyer([CSI, str(nb_colonne), 'P'])

        if nb_ligne != None:
            self.envoyer([CSI, str(nb_ligne), 'M'])

    def insere(self, nb_colonne = None, nb_ligne = None):
        """Insère des caractères après le curseur

        En insérant des caractères après le curseur, le Minitel pousse les
        derniers caractères contenus sur la ligne à droite.

        :param nb_colonne:
            nombre de caractères à insérer
        :type nb_colonne:
            un entier positif
        :param nb_ligne:
            nombre de lignes à insérer
        :type nb_ligne:
            un entier positif
        """
        assert (isinstance(nb_colonne, int) and nb_colonne >= 0) or \
                nb_colonne == None
        assert (isinstance(nb_ligne, int) and nb_ligne >= 0) or \
                nb_ligne == None

        if nb_colonne != None:
            self.envoyer([CSI, '4h', ' ' * nb_colonne, CSI, '4l'])

        if nb_ligne != None:
            self.envoyer([CSI, str(nb_ligne), 'L'])

    def semigraphique(self, actif = True):
        """Passe en mode semi-graphique ou en mode alphabétique

        :param actif:
            True pour passer en mode semi-graphique, False pour revenir au
            mode normal
        :type actif:
            un booléen
        """
        assert actif in [True, False]

        actifs = { True: SO, False: SI}
        self.envoyer(actifs[actif])

    def redefinir(self, depuis, dessins, jeu = 'G0'):
        """Redéfinit des caractères du Minitel

        À partir du Minitel 2, il est possible de redéfinir des caractères.
        Chaque caractère est dessiné à partir d’une matrice 8×10 pixels.

        Les dessins des caractères sont données par une suite de 0 et de 1 dans
        une chaîne de caractères. Tout autre caractère est purement et
        simplement ignoré. Cette particularité permet de dessiner les
        caractères depuis un éditeur de texte standard et d’ajouter des
        commentaires.
        
        Exemple::

            11111111
            10000001
            10000001
            10000001
            10000001 Ceci est un rectangle !
            10000001
            10000001
            10000001
            10000001
            11111111

        Le Minitel n’insère aucun pixel de séparation entre les caractères,
        il faut donc prendre cela en compte et les inclure dans vos dessins.

        Une fois le ou les caractères redéfinis, le jeu de caractères spécial
        les contenant est automatiquement sélectionné et ils peuvent donc
        être utilisés immédiatement.

        :param depuis:
            caractère à partir duquel redéfinir
        :type depuis:
            une chaîne de caractères
        :param dessins:
            dessins des caractères à redéfinir
        :type dessins:
            une chaîne de caractères
        :param jeu:
            palette de caractères à modifier (G0 ou G1)
        :type jeu:
            une chaîne de caractères
        """
        assert jeu == 'G0' or jeu == 'G1'
        assert isinstance(depuis, str) and len(depuis) == 1
        assert isinstance(dessins, str)

        # Deux jeux sont disponible G’0 et G’1
        if jeu == 'G0':
            self.envoyer([US, 0x23, 0x20, 0x20, 0x20, 0x42, 0x49])
        else:
            self.envoyer([US, 0x23, 0x20, 0x20, 0x20, 0x43, 0x49])

        # On indique à partir de quel caractère on veut rédéfinir les dessins
        self.envoyer([US, 0x23, depuis, 0x30])

        octet = ''
        compte_pixel = 0
        for pixel in dessins:
            # Seuls les caractères 0 et 1 sont interprétés, les autres sont
            # ignorés. Cela permet de présenter les dessins dans le code
            # source de façon plus lisible
            if pixel != '0' and pixel != '1':
                continue

            octet = octet + pixel
            compte_pixel += 1

            # On regroupe les pixels du caractères par paquets de 6
            # car on ne peut envoyer que 6 bits à la fois
            if len(octet) == 6:
                self.envoyer(0x40 + int(octet, 2))
                octet = ''

            # Quand 80 pixels (8 colonnes × 10 lignes) ont été envoyés
            # on ajoute 4 bits à zéro car l’envoi se fait par paquet de 6 bits
            # (8×10 = 80 pixels, 14×6 = 84 bits, 84-80 = 4)
            if compte_pixel == 80:
                self.envoyer(0x40 + int(octet + '0000', 2))
                self.envoyer(0x30)
                octet = ''
                compte_pixel = 0

        # Positionner le curseur permet de sortir du mode de définition
        self.envoyer([US, 0x41, 0x41])

        # Sélectionne le jeu de caractère fraîchement modifié (G’0 ou G’1)
        if jeu == 'GO':
            self.envoyer([ESC, 0x28, 0x20, 0x42])
        else:
            self.envoyer([ESC, 0x29, 0x20, 0x43])

