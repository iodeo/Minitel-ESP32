////////////////////////////////////////////////////////////////////////
/*
   Minitel1B_Hard - Démo - Version du 11 juin 2017 à 16h00
   Copyright 2016, 2017 - Eric Sérandour
   
   >> Légèrement adapté pour l'ESP32 par iodeo
   
   Documentation utilisée :
   Spécifications Techniques d'Utilisation du Minitel 1B
   http://543210.free.fr/TV/stum1b.pdf
   
////////////////////////////////////////////////////////////////////////
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

// DEBUT DU PROGRAMME

////////////////////////////////////////////////////////////////////////


#include <Minitel1B_Hard.h>

Minitel minitel(Serial2);  // Le port utilisé sur ESP32

int wait = 10000;

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);  // Le port de débug
  minitel.changeSpeed(minitel.searchSpeed());
}

////////////////////////////////////////////////////////////////////////

void loop() {
  demoCaracteres();
  demoGraphic();
  demoTailles();
  demoCouleurs();
  demoCurseur();
}

////////////////////////////////////////////////////////////////////////

void newPage(String titre) {
  minitel.newScreen();
  minitel.println(titre);
  for (int i=1; i<=40; i++) {
    minitel.writeByte(0x7E);
  }
  minitel.moveCursorReturn(1); 
}

////////////////////////////////////////////////////////////////////////

void demoCaracteres() {
  newPage("LES CARACTERES");
  
  // Mode texte 
  
  minitel.println("MODE TEXTE SANS LIGNAGE :");
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.moveCursorReturn(2);
  
  minitel.println("MODE TEXTE AVEC LIGNAGE :");
  minitel.attributs(DEBUT_LIGNAGE);  // En mode texte, le lignage est déclenché par le premier espace rencontré (0x20).
  for (int i=0x20; i<=0x7F; i++) {   
    minitel.writeByte(i);   
  }
  minitel.attributs(FIN_LIGNAGE);
  minitel.moveCursorReturn(2);  

  // Mode semi-graphique
  
  minitel.textMode();
  minitel.println("MODE SEMI-GRAPHIQUE SANS LIGNAGE :");
  minitel.graphicMode();  
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.moveCursorReturn(2);
  
  minitel.textMode();
  minitel.println("MODE SEMI-GRAPHIQUE AVEC LIGNAGE :");
  minitel.graphicMode();
  minitel.attributs(DEBUT_LIGNAGE);
  for (int i=0x20; i<=0x7F; i++) {
    minitel.writeByte(i);
  }
  minitel.attributs(FIN_LIGNAGE);
  minitel.moveCursorReturn(2);
    
  delay(wait); 
}

////////////////////////////////////////////////////////////////////////

void demoGraphic() {
  newPage("LA FONCTION GRAPHIC");
  minitel.textMode();
  minitel.println("Un caractère semi-graphique est composé de 6 pseudo-pixels :");
  minitel.println();
  minitel.graphicMode();
  minitel.attributs(DEBUT_LIGNAGE);
  minitel.writeByte(0x7F);
  minitel.attributs(FIN_LIGNAGE);
  minitel.textMode();
  minitel.print(" avec lignage ou ");
  minitel.graphicMode();
  minitel.writeByte(0x7F);
  minitel.textMode();
  minitel.println(" sans lignage.");
  minitel.println();
  String chaine = "";
  chaine += "minitel.graphic(0b101011) donne ";
  minitel.textMode();
  minitel.print(chaine);
  minitel.graphicMode();
  minitel.graphic(0b101011);
  minitel.textMode();
  minitel.println();
  minitel.println();
  chaine = "";
  chaine += "minitel.graphic(0b110110,30,15) donne ";
  minitel.print(chaine);  
  minitel.graphicMode();
  minitel.graphic(0b110110,30,15);
  minitel.noCursor();
  delay(2*wait); 
}

////////////////////////////////////////////////////////////////////////

void demoTailles() { 
  newPage("LES TAILLES");
  minitel.println("GRANDEUR_NORMALE");
  minitel.attributs(DOUBLE_HAUTEUR);
  minitel.print("DOUBLE_HAUTEUR");  
  minitel.attributs(DOUBLE_LARGEUR);
  minitel.println();
  minitel.println("DOUBLE_LARGEUR");  
  minitel.attributs(DOUBLE_GRANDEUR); 
  minitel.println("DOUBLE_GRANDEUR");
  minitel.println();
  minitel.attributs(GRANDEUR_NORMALE);
  minitel.attributs(DEBUT_LIGNAGE);  // En mode texte, le lignage est déclenché par le premier espace rencontré (0x20).
  minitel.println("SEULEMENT EN MODE TEXTE");  
  minitel.attributs(FIN_LIGNAGE);
  minitel.println();
  delay(wait);  
}

////////////////////////////////////////////////////////////////////////

void demoCouleurs() {
  newPage("LES COULEURS");
  for (int i=0; i<=1; i++) {   
    if (i==0) { minitel.textMode(); }
    if (i==1) { minitel.graphicMode(); }
    minitel.attributs(INVERSION_FOND);
    minitel.print("CARACTERE_NOIR, FOND_BLANC");
    minitel.attributs(FOND_NORMAL);
    minitel.println(" (INVERSION)");      
    minitel.attributs(CARACTERE_ROUGE);
    minitel.println("CARACTERE_ROUGE");
    minitel.attributs(CARACTERE_VERT);
    minitel.println("CARACTERE_VERT");
    minitel.attributs(CARACTERE_JAUNE);
    minitel.println("CARACTERE_JAUNE");
    minitel.attributs(CARACTERE_BLEU);
    minitel.println("CARACTERE_BLEU");
    minitel.attributs(CARACTERE_MAGENTA);
    minitel.println("CARACTERE_MAGENTA");
    minitel.attributs(CARACTERE_CYAN);
    minitel.println("CARACTERE_CYAN");
    minitel.attributs(CARACTERE_BLANC);
    minitel.println("CARACTERE_BLANC");
    minitel.println();
  }
  delay(wait);
}

////////////////////////////////////////////////////////////////////////

void demoCurseur() {
  minitel.cursor();
  newPage("DEPLACER LE CURSEUR");
  minitel.moveCursorXY(20,12);
  for (int i=1; i<=100; i++) {
    delay(100);
    switch (random(4)) {
      case 0: minitel.moveCursorRight(1+random(3)); break;
      case 1: minitel.moveCursorLeft(1+random(3)); break;
      case 2: minitel.moveCursorDown(1+random(3)); break;
      case 3: minitel.moveCursorUp(1+random(3)); break;
    }
  }
  newPage("POSITIONNER LE CURSEUR");
  minitel.textMode();
  for (int i=1; i<1000; i++) {
    minitel.moveCursorXY(1+random(40),3+random(22));
    minitel.writeByte(0x20 + random(0x60));
  }
  
  minitel.newScreen();
  minitel.textMode();
  minitel.noCursor();
  for (int i=1; i<1000; i++) {
    if (random(4)<3) { minitel.textMode(); }
    else {
      minitel.graphicMode();
      minitel.attributs(DEBUT_LIGNAGE);
    }
    minitel.attributs(0x4C+random(5));
    minitel.writeByte(0x20 + random(0x60));
    minitel.attributs(FIN_LIGNAGE);
  }
}

////////////////////////////////////////////////////////////////////////
