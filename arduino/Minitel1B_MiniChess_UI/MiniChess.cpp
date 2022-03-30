#include "MiniChess.h"

// ---------------------
//   Public functions 
// ---------------------

MiniChess::MiniChess(HardwareSerial& serial) : minitel(serial) {
  // Constructeur: on construit juste l'instance minitel
}

//----------------------------------------------------------

void MiniChess::initializeMinitel(int bauds) {
  // On etablit la liaison série à la vitesse voulue et 
  // on configure le mninitel pour le jeu
  if (minitel.searchSpeed() != bauds)
    minitel.changeSpeed(bauds);
  minitel.echo(false); // désactivation de l'écho local
  minitel.extendedKeyboard(); // activation des touches curseurs
  minitel.clearScreen(); // effacement écran
  minitel.noCursor(); // le curseur natif du minitel est désactivé
  minitel.attributs(FIXE); // Pas de clignotement
}

//----------------------------------------------------------

void MiniChess::drawChessBoard() {
  // Fonction d'affichage du plateau de jeu seul
  
  // on se place en haut à gauche sur la case A8 qui est blanche
  minitel.moveCursorXY(BOARD_LEFT, BOARD_TOP);
  bool dark = false; //couleur de la case
  int cy = 8; // numero de case en ordonnées
  // on boucle sur les cases de haut en bas
  while (cy > 0) {
    int row = 1;
    // on boucle sur les 3 lignes constituant une case
    while (row <= CASE_HEIGHT) {
      int cx = 1; // numero de case en abscisses
      // on boucle sur les cases de gauche à droite
      while (cx < 9) {
        // on passe en mode texte pour afficher le nom de la case
        minitel.textMode();
        // on règle la couleur de caractère que l'on passe en arrière plan ensuite
        if (dark) minitel.attributs(CARACTERE_BLEU);
        else minitel.attributs(CARACTERE_VERT);
        minitel.attributs(INVERSION_FOND);
        if (row == 1) minitel.printChar(cx+64);    // A-H
        if (row == 2) minitel.printChar(cy+48);    // 1-8
        if (row == 3) minitel.printChar(SP);       // ESPACE
        // on passe en mode semi-graphique
        minitel.graphicMode();
        // on règle la couleur de la case
        if (dark) minitel.attributs(FOND_BLEU);
        else minitel.attributs(FOND_VERT);
        // on dessine un carcatère vide
        minitel.graphic(0b000000);
        // qu'on répète 2 fois
        minitel.repeat(CASE_WIDTH - 2);
        dark = !dark; // changement de couleur de case
        cx++; // case suivante
      }
      minitel.moveCursorLeft(CASE_WIDTH*8);
      minitel.moveCursorDown(1);
      row++; // ligne suivante
    }
  dark = !dark; // changement de couleur de case
  cy--; // rangée suivante
  }
}

//---------------------------------------------------------------

void MiniChess::initPiecesPosition() {
  // Fonction d'initialisation de la position des pièces sur le plateau
  for (int i = 0; i < 5; i++) board[i][0] = (i+2)   + _BLACK;
  for (int i = 5; i < 8; i++) board[i][0] = (9-i) + _BLACK;
  for (int i = 0; i < 8; i++) board[i][1] = PAWN    + _BLACK;
  for (int j = 2; j < 6; j++) {
    for (int i = 0; i < 8; i++) board[i][j] = VOID;
  }
  for (int i = 0; i < 5; i++) board[i][7] = (i+2)   + _WHITE;
  for (int i = 5; i < 8; i++) board[i][7] = (9-i) + _WHITE;
  for (int i = 0; i < 8; i++) board[i][6] = PAWN    + _WHITE;
}


//---------------------------------------------------------------

void MiniChess::drawAllPieces() {
  // Fonction pour afficher toutes les pièces en début de jeu
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (j == 2) j = 6; // on saute les lignes vides
      drawPiece(i, j, board[i][j]);
    }
  }
  // on affiche le curseur de selection
  markCase(cx, cy, true);
}

//---------------------------------------------------------------

void MiniChess::getMove() {
  // Fonction pour acquérir le prochain coup
  bool gotMove = false;
  
  while (!gotMove) {
    // Saisie clavier
    unsigned long c = getKeyboardInput();
    if (c != 0) {
      switch (c) {
        // Mouvement du curseur
        case TOUCHE_FLECHE_HAUT:   moveUp();    break;
        case TOUCHE_FLECHE_BAS:    moveDown();  break;
        case TOUCHE_FLECHE_GAUCHE: moveLeft();  break;
        case TOUCHE_FLECHE_DROITE: moveRight(); break;
        // Selection
        case CR:
          // Case de départ
          if (scx == -1) {
            if (validateSelection()) {
                markCase(cx, cy, true); // marqueur de selection
            }
          }
          // Case d'arrivée
          else {
            if (validateMove()) {
              gotMove = true;
            }
          }
          break;
        // Annulation
        case CAN:
          // S'il y a une selection en cours
          if (scx != -1) {
            // on efface le marqueur de selection
            markCase(scx, scy, false, true);
            // on affiche le marqueur de position si besoin
            if (scx == cx && scy == cy)
              markCase(cx, cy, true);
            scx = -1; scy = -1;
          }
          break;
      }
    }
  }
}

//---------------------------------------------------------------

void MiniChess::makeMove() {
  // Fonction pour afficher le mouvement de pièces
  // en gérant les mouvements spéciaux
  drawMove();
  byte b = board[cx][cy];
  // Petit roque  et Grand roque
  if ((b & 0b01111111) == KING) {
    if ((b & COLOR_MASK) == _BLACK) {
      Serial.println("BLACK");
      if (scx == 4 && scy == 0) {
        if (cx == 1 && cy == 0) {
          Serial.println("ROQUE A GAUCHE");
          // roque des noirs à gauche
          markCase(cx, cy, false);
          scx = 0; scy = 0;
          cx = 2; cy = 0;
          drawMove();
        }
        if (cx == 6 && cy == 0) {
          // roque des noirs à droite
          markCase(cx, cy, false);
          scx = 7; scy = 0;
          cx = 5; cy = 0;
          drawMove();
        }
      }
    }
    if ((b & COLOR_MASK) == _WHITE) {
      if (scx == 4 && scy == 7) {
        if (cx == 1 && cy == 7) {
          // roque des blancs à gauche
          markCase(cx, cy, false);
          scx = 0; scy = 7;
          cx = 2; cy = 7;
          drawMove();
        }
        if (cx == 6 && cy == 7) {
          // roque des blancs à droite
          markCase(cx, cy, false);
          scx = 7; scy = 7;
          cx = 5; cy = 7;
          drawMove();
        }
      }
    }
  }
  // Promotion de pion - limité à la Reine
  if ((b & PIECE_MASK) == PAWN) {
    if ((b & COLOR_MASK) == _BLACK) {
      if (cy == 7) {
        // promotion d'un pion noir
        byte pc = QUEEN + _BLACK;
        board[cx][cy] = pc;
        drawPiece(cx, cy, pc);
      }
    }
    if ((b & COLOR_MASK) == _WHITE) {
      if (cy == 0) {
        // promotion d'un pion blanc
        byte pc = QUEEN + _WHITE;
        board[cx][cy] = pc;
        drawPiece(cx, cy, pc);
      }
    }
  }
  // on reinitialise la selection
  scx = -1; scy = -1;
}

//---------------------------------------------------------------

void MiniChess::changePlayer() {
  // Fonction pour changer de joueur
  if (player == _WHITE) player = _BLACK;
  else player = _WHITE;
  // le marqueur change de couleur
  markCase(cx, cy, true);
}

//---------------------------------------------------------------

// ---------------------
//   Private functions 
// ---------------------

unsigned long MiniChess::getKeyboardInput() {
  // Fonction pour rediriger les saisies du clavier
  
  unsigned long key = minitel.getKeyCode();
  switch (key) {
    
    // Annulation de la sélection en cours
    case CORRECTION:
    case ANNULATION:
    case RETOUR:
    case ESC:
    case DEL:
                               return CAN;   break;
    
    // Sélection
    case ENVOI:
    case SP:
                               return CR;    break;
    
    // Touches inhibées
    case CONNEXION_FIN:
    case SOMMAIRE:
    case REPETITION:
    case GUIDE:
    case SUITE:
                               return 0;     break;
    
    // Autres touches
    default:
                               return key;   break;
    
  }
}

//---------------------------------------------------------------

void MiniChess::moveUp() {
  // Fonction pour déplacer la postion courante vers le haut
  if (cy > 0) {
    markCase(cx,cy, false);
    cy--;
    markCase(cx,cy, true);
  }
}

//---------------------------------------------------------------

void MiniChess::moveDown() {
  // Fonction pour déplacer la postion courante vers le bas
  if (cy < 7) {
    markCase(cx,cy, false);
    cy++;
    markCase(cx,cy, true);
  }
}

//---------------------------------------------------------------

void MiniChess::moveLeft() {
  // Fonction pour déplacer la postion courante vers la gauche
  if (cx > 0) {
    markCase(cx,cy, false);
    cx--;
    markCase(cx,cy, true);
  }
}

//---------------------------------------------------------------

void MiniChess::moveRight() {
  // Fonction pour déplacer la postion courante vers la droite
  if (cx < 7) {
    markCase(cx,cy, false);
    cx++;
    markCase(cx,cy, true);
  }
}

//---------------------------------------------------------------

void MiniChess::drawPiece(int cx, int cy, byte pc) {
  // Fonction pour afficher une pièce dans une case donnée
  // avec pc: l'identifiant de la pièce et de la couleur
  
  // on détermine les coordonnées du curseur du minitel
  int x = cx * CASE_WIDTH + BOARD_LEFT;
  int y = cy * CASE_HEIGHT + BOARD_TOP;
  
  // on récupère la couleur et la pièce
  byte color = pc & COLOR_MASK;
  byte p = pc & PIECE_MASK;
  
  // on passe en mode graphique et on règle les couleurs
  minitel.graphicMode();
  if (color == _WHITE) {
    // effet lignage pour améliorer la netteté
    minitel.attributs(DEBUT_LIGNAGE);
    minitel.attributs(CARACTERE_BLANC);
  }
  else { // _BLACK
    minitel.attributs(CARACTERE_NOIR);
  }
  // on détermine la couleur de la case
  if ((cx+cy)%2 == 1) minitel.attributs(FOND_BLEU);
  else minitel.attributs(FOND_VERT);
  // on dessine la pièce caractère par caractère
  for (int j = 0; j < PIECE_HEIGHT; j++) {
    minitel.moveCursorXY(x+1,y+j);
    for (int i = 0; i < PIECE_WIDTH; i++) {
      minitel.graphic(piece[p][i+j*PIECE_WIDTH]);
    }
  }
  if (color == _WHITE) {
    minitel.attributs(FIN_LIGNAGE);
  }
}

//---------------------------------------------------------------

void MiniChess::erasePiece(int cx, int cy) {
  // Fonction pour effacer une case donnée
  drawPiece(cx, cy, VOID);
}

//---------------------------------------------------------------

void MiniChess::markCase(int cx, int cy, bool mark, bool force_erase) {
  // Fonction pour marqué la case courante dans le coin bas-gauche
  // Si les blancs jouent, la marque de position est blanche
  // et la marque de selection est noire
  // Et inversement pour les noirs
  // Si mark = true, la marque de position ou de selection est affichée
  // Si mark = false, la marque est effacée sauf s’il s’agit de la selection
  // auquel cas il faut mettre force_erase = true pour l’effacer

  // on place le curseur du minitel au bon endroit
  int x = cx*CASE_WIDTH + BOARD_LEFT;
  int y = cy*CASE_HEIGHT + BOARD_TOP + 2;
  minitel.moveCursorXY(x,y);
  // on se met en mode graphique
  minitel.graphicMode();
  // on détermine si on est surune case sombre ou claire
  bool dark = false;
  if ((cx+cy)%2 == 1) dark = true;
  // on ajuste la couleur de fond
  if (dark) minitel.attributs(FOND_BLEU);
  else minitel.attributs(FOND_VERT);
  // on détermine si on est sur la case sélectionnée 
  bool selected = (cx == scx && cy == scy);
  if (mark) {
    // on détermine la couleur à mettre
    byte color = CARACTERE_BLANC;
    if (player == _WHITE)
      if (selected) color = CARACTERE_NOIR;
    if (player == _BLACK) {
      if (!selected) color = CARACTERE_NOIR;
    }
    // on affiche la marque
    minitel.attributs(color);
    minitel.graphic(0b111111);
  }
  else {
    if (!selected || force_erase)
      // on efface la marque
      minitel.graphic(0b000000);
  }
}

//---------------------------------------------------------------

void MiniChess::markCase(int cx, int cy, bool mark) {
  // force_erase = false par défaut
  markCase(cx, cy, mark, false);
}

//---------------------------------------------------------------

bool MiniChess::validateSelection() {
  byte b = board[cx][cy];
  // La case ne contient pas de pièce
  if ((b & PIECE_MASK) == VOID) return false;
  // La pièce est à l'adversaire
  if ((b & COLOR_MASK) != player) return false;
  // La sélection est valide, on la récupère
  scx = cx; scy = cy;
  return true;
}

//---------------------------------------------------------------

bool MiniChess::validateMove() {
  // La case d'arrivée est la même que la case de départ
  if (cx == scx && cy == scy) return false;
  // TODO : verifier si le mouvement est réglementaire
  return true;  
}

//---------------------------------------------------------------

void MiniChess::drawMove() {
  // Fonction pour afficher un mouvement de pièce
  byte b = board[scx][scy];
  board[cx][cy] = b;
  board[scx][scy] = VOID;
  // on efface la pièce de la case d'origine
  erasePiece(scx, scy);
  // on retire le marqueur de sélection
  markCase(scx, scy, false, true);
  // on dessine la pièce à son nouvel emplacement
  drawPiece(cx, cy, board[cx][cy]);
  // on retire le marqueur de position
  markCase(cx, cy, false);
}

//---------------------------------------------------------------
