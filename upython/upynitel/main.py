#!/usr/bin/env python3

from machine import UART
import upynitel

#from bs4 import BeautifulSoup
#import requests
#import sys
#import json

import network
import urequests
import ujson

m = None

def do_connect():

    # wifi credientials
    ssid = "YOUR_WIFI_NETWORK_SSID"
    password = "YOUR_WIFI_NETWORK_PASSWORD"
    
    # connect in station mode
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            pass
    print('network config:', wlan.ifconfig())


def init():
    "Initialisation du serveur vidéotex"
    global m
    m = upynitel.Pynitel(UART(2, baudrate = 1200,parity=0, bits=7,stop=1))
    (quoi, ou) = ('', '')
    return(quoi, ou)


def annuaire_saisie(quoi, ou):
    "Masque de saisie des critères de recherche"
    # définition des zones
    global m
    m.resetzones()
    m.zone(5, 13, 27, quoi, m.blanc)
    m.zone(7, 13, 27, '', m.blanc)
    m.zone(10, 13, 27, ou, m.blanc)
    m.zone(13, 13, 27, '', m.blanc)
    m.zone(14, 13, 27, '', m.blanc)
    m.zone(15, 13, 27, '', m.blanc)
    touche = m.repetition
    zone = 1

    while True:
        # affichage initial ou répétition
        if touche == m.repetition:
            m.home()
            m.xdraw('ecrans/E.ANNUAIRE.OPTIM.vtx')

        # gestion de la zone de saisie courante
        (zone, touche) = m.waitzones(zone)

        if touche != m.repetition:
            break

    quoi = ("%s %s %s" % (m.zones[0]['texte'], m.zones[1]['texte'],
                          m.zones[5]['texte'])).strip()
    ou = ("%s %s %s" % (m.zones[4]['texte'], m.zones[3]['texte'],
                        m.zones[2]['texte'])).strip()
    return (touche, quoi, ou)


def annuaire_recherche(quoi, ou):
    "Effectue une recherche sur plusieurs annuaires"
    res = []
#     if len(res) == 0:
#         annu = "118712.fr"
#         res = annuaire118712(quoi, ou)
#     if len(res) == 0:
#         annu = "118218.fr"
#         res = annuaire118218(quoi, ou)
#     if len(res) == 0:
#         annu = "118000.fr"
#         res = annu118000(quoi, ou)
    if True:
        annu = "TestJsonApi"
        res = testjsonapi()
    return(res, annu)

def testjsonapi():
    res = []
    req = urequests.get('https://www.testjsonapi.com/users/')
    parsed = ujson.loads(req.content)
    
    for p in parsed:
        nom=p['name']
        cp=p['created_at']
        ville=''
        tel=p['job_title']
        adresse=p['email']        
        result = dict(nom=nom, cp=cp, ville=ville, tel=tel, adresse = adresse)
        res.append(result)
        
    return(res)

def add_if_not_none(the_dict, key, item):
    if item is not None:
        the_dict[key] = item.string.strip()


def annuaire118712(qui, ou):
    # recherche sur l'annuaire d'Orange (mais problème de captcha)
    res = []
    req = requests.get('https://annuaire.118712.fr/', params={"s": qui+' '+ou})
    h = BeautifulSoup(req.text, 'lxml')
    for p in h.find_all(itemtype="http://schema.org/Person"):
        nom = p.find(itemprop="name").a.string.strip()
        cp = p.find(itemprop="postalCode").string.strip()
        ville = p.find(itemprop="addressLocality").string.strip()
        tel = p.find(itemprop="telephone").string.strip()
        propart = p.find(class_="propart_text").string.strip()
        result = dict(nom=nom, cp=cp, ville=ville, tel=tel, propart=propart)
        adresse = p.find(itemprop="streetAddress")
        if adresse is not None:
            result['adresse'] = adresse.string.strip()
        else:
            result['adresse'] = ''
        lat = p.find(itemprop="latitude")
        if lat is not None:
            result['lat'] = lat.string.strip()
        lon = p.find(itemprop="longitude")
        if lon is not None:
            result['lon'] = lon.string.strip()
        categories = p.find(class_="categories")
        if categories is not None:
            result['categories'] = categories.string.strip()
        res.append(result)
    for p in h.find_all(itemtype="http://schema.org/LocalBusiness"):
        nom = p.find(itemprop="name").a.string.strip()
        result = dict(nom=nom)
        add_if_not_none(result, 'adresse', p.find(itemprop="streetAddress"))
        add_if_not_none(result, 'cp', p.find(itemprop="postalCode"))
        add_if_not_none(result, 'ville', p.find(itemprop="addressLocality"))
        add_if_not_none(result, 'tel', p.find(itemprop="telephone"))
        if 'tel' not in result:
            tel = p.find(class_="hidden-phone")
            if tel is not None:
                result['tel'] = tel['data-wording']
        add_if_not_none(result, 'categories', p.find(class_="categories"))
        res.append(result)
    return(res)


def annuaire118218(qui, ou):
    # recherche sur l'annuaire 118218 (pas de captcha ?)
    res = []
    # recherche particuliers
    req = requests.get('http://www.118218.fr/recherche',
                       params={"who": qui, "where": ou})
    h = BeautifulSoup(req.text, 'lxml')
    for p in h.find_all("section", class_="searchResult"):
        if p.a is not None:
            adresse = p.address.span.string.replace(',', '')
            p.address.span.unwrap()
            cp = p.address.span.span.string.replace(',', '')
            p.address.span.unwrap()
            p.address.span.unwrap()
            ville = p.address.span.string.replace('\n', '').strip()
            res.append(dict(nom=p.a.string, adresse=adresse, cp=cp,
                            ville=ville, tel=p.p.string))
    # recherche pro
    req = requests.get('http://www.118218.fr/recherche',
                       params={"what": qui, "where": ou})
    h = BeautifulSoup(req.text, 'lxml')
    for p in h.find_all("section", class_="searchResult"):
        if p.a is not None:
            adresse = p.address.span.string.replace(',', '')
            p.address.span.unwrap()
            cp = p.address.span.span.string.replace(',', '')
            p.address.span.unwrap()
            p.address.span.unwrap()
            ville = p.address.span.string.replace('\n', '').strip()
            res.append(dict(nom=p.a.string, adresse=adresse,
                            cp=cp, ville=ville, tel=p.p.string))
    return(res)


def annu118000(qui, ou):
    # recherche sur l'annuaire 118000 (pas de captcha ?)
    res = []
    # recherche particuliers
    req = requests.get('https://www.118000.fr/search',
                       params={"who": qui, "label": ou})
    h = BeautifulSoup(req.text, 'lxml')
    for p in h.find_all(class_="card"):
        b = p.find(class_="iconheart")
        if b['data-info'] is not None:
            j = json.loads(b['data-info'])
            res.append(dict(nom=p.h2.a.string, adresse=j['address'],
                            cp=j['cp'], ville=j['city'], tel=j['tel']))
    return(res)


def strformat(left='', right='', fill=' ', width=40):
    " formattage de texte "
    total = width-len(left+right)
    if total > 0:
        out = left + fill * total + right
    else:
        out = left+right
    print("'"+out+"'", width, total, len(out))
    return(out)


def affiche_resultat(quoi, ou, res, annu=''):
    "Affiche les résultats de la recherche"
    global m
    page = 1
    while True:
        if page > 0:  # affichage
            # entête sur 2 lignes + séparation
            m.home()
            m._print(quoi.upper()+' à '+ou)
            m.pos(2)
            m.color(m.bleu)
            m.plot('̶', 40)
            if annu != '':
                m.pos(23, 1)
                m.color(m.bleu)
                m._print("(C)\x0d\x0a"+annu)

            # plusieurs pages ?
            if len(res) > 5:
                m.pos(1, 37)
                m._print(" "+str(int(abs(page)))+'/'+str(int((len(res)+4)/5)))
                m.pos(3)

            # if len(res)>5:
            #     if abs(page)>1:
            #         m.pos(2,33)
            #         m.inverse()
            #         m.color(m.cyan)
            #         m._print('↑RETOUR↑')
            #         m.inverse(False)
            #     else:
            #         m.pos(2,33)
            #         m.color(m.bleu)
            #         m.plot('̶', 8)

            # première ligne de résultat
            m.pos(3)
            for a in range((page-1)*5, page*5):
                if a < len(res):
                    r = res[a]
                    if 'adresse' not in r or r['adresse'] == '':
                        r['adresse'] = '(adresse masquée)'
                    if 'tel' not in r or r['tel'] == '':
                        r['tel'] = ' (num. masqué)'
                    m.color(m.blanc)
                    m._print(strformat(right=str(int(a+1)), width=3))
                    m._print(' '+strformat(left=r['nom'][:20],
                                           right=r['tel'], width=36))
                    m.color(m.vert)
                    m._print('    '+r['adresse'][:35]+'\x0d\x0a    ' +
                             r['cp']+' '+r['ville']+'\x0d\x0a')
                    m.color(m.bleu)
                    if a < page*5:
                        m.plot(' ', 4)
                        m.plot('̶', 36)

            # ligne finale
            m.pos(22)
            m.color(m.bleu)
            m.plot('̶', 40)

            # if len(res)>5:
            #     if len(res)>page*5:
            #         m.pos(22,34)
            #         m.inverse()
            #         m.color(m.cyan)
            #         m._print('↓SUITE↓')
            #     else:
            #         m.pos(22,33)
            #         m.color(m.bleu)
            #         m.plot('̶', 8)
            # if len(res)<5 or len(res)<=page*5:
            #     m.pos(22)
            #     m.color(m.bleu)
            #     m.plot('̶', 40)

            if page > 1:
                if len(res) > page*5:  # place pour le SUITE
                    m.pos(22, 15)
                else:
                    m.pos(23, 15)
                m.color(m.vert)
                m._print('page précédente →')
                m.underline()
                m._print(' ')
                m.inverse()
                m.color(m.cyan)
                m._print('_RETOUR ')
            if len(res) > page*5:
                m.pos(23, 17)
                m.color(m.vert)
                m._print('page suivante →')
                m.underline()
                m._print(' ')
                m.inverse()
                m.color(m.cyan)
                m._print('_SUITE  ')

            m.pos(24, 15)
            m.color(m.vert)
            m._print("autre recherche → ")
            m.inverse()
            m.color(m.cyan)
            m._print("SOMMAIRE")
        else:
            page = abs(page)

        # attente saisie
        (choix, touche) = m.input(0, 1, 0, '')
        m.cursor(False)
        if touche == m.suite:
            if page*5 < len(res):
                page = page + 1
            else:
                m.bip()
                page = -page  # pas de ré-affichage
        elif touche == m.retour:
            if page > 1:
                page = page - 1
            else:
                m.bip()
                page = -page  # pas de ré-affichage
        elif touche == m.sommaire:
            break
        elif touche == m.correction:  # retour saisie pour correction
            return(touche)
        elif touche != m.repetition:
            m.bip()
            page = -page  # pas de ré-affichage

    return(touche)


def annuaire():
    global m
    (annuaire_quoi, annuaire_ou) = init()

    while True:
        print(annuaire_quoi, annuaire_ou)
        (touche, annuaire_quoi, annuaire_ou) = annuaire_saisie(annuaire_quoi,
                                                               annuaire_ou)
        if touche == m.envoi:
            # on lance la recherche
            m.cursor(False)
            m.pos(0, 1)
            m.flash()
            m._print('Recherche... ')
            (resultat, annu) = annuaire_recherche(annuaire_quoi, annuaire_ou)
            print(resultat)
            if len(resultat) == 0:
                m.message(0, 1, 3, "Aucune adresse trouvée")
            else:
                if affiche_resultat(annuaire_quoi, annuaire_ou,
                                    resultat, annu) != m.correction:
                    (annuaire_quoi, annuaire_ou) = ('', '')

do_connect()
annuaire()
