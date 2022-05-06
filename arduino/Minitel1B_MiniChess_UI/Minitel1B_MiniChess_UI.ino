#include "Minichess.h"
#include <Minitel1B_Hard.h>

#define MINITEL_PORT Serial2 // For ESP32 dev board
//#define MINITEL_PORT Serial1 // For ATMEGA32u4
#define BAUDS 4800

MiniChess chess(MINITEL_PORT);

void setup() {
  // on attend que le minitel démarre
  delay(500);
  // réglage port série et config minitel
  chess.initializeMinitel(BAUDS); 

  // initialisation du jeu
  chess.drawChessBoard();     // affichage plateau vide
  chess.initPiecesPosition(); // placement des pieces
  chess.drawAllPieces();      // affichage des pièces
}

void loop() {
  while (true) {
    chess.getMove();
    chess.makeMove();
    chess.changePlayer();
  }
}
